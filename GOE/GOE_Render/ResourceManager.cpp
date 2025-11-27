#include "Renderer_pch.h"
#include "ResourceManager.h"

#include <d3dx12/d3dx12.h>
#include "DirectXTex.h"	

#include "CopyCommandContext.h"
#include "DescriptorHeapManager.h"
#include "RenderCommandContext.h"
#include "PSOManager.h"

// 리소스자료형
#include "MeshResource.h"
#include "TextureResource.h"
#include "RenderObject.h"

Graphics::ResourceManager::~ResourceManager() = default;

void Graphics::ResourceManager::Initialize(RenderContext* renderContext)
{
	m_renderContext = renderContext;
}

/// <summary>
/// 리소스 상태를 확인, 전환하는 함수
/// 
/// </summary>
void Graphics::ResourceManager::UpdateResourceStates()
{
	const auto device = m_renderContext->m_graphicsDevice;
	const UINT64 completedFenceValue = device->GetCompletedCopyFenceValue();
	
	auto textureIt = std::remove_if
	(m_loadingTextures.begin(), m_loadingTextures.end(),
		[completedFenceValue](const auto& pair)
		{
			if (pair.first <= completedFenceValue)
			{
				if(auto textureResource = pair.second.lock())
				{
					textureResource->SetState(Graphics::ResourceState::READY);
				}
				return true; 
			}
			return false;
		});
	m_loadingTextures.erase(textureIt, m_loadingTextures.end());

	auto meshIt = std::remove_if
	(m_loadingMeshes.begin(), m_loadingMeshes.end(),
		[completedFenceValue](const auto& pair)
		{
			if (pair.first <= completedFenceValue)
			{
				if(auto meshResource = pair.second.lock())
				{
					meshResource->SetState(Graphics::ResourceState::READY);
				}
				return true; 
			}
			return false;
		});
	m_loadingMeshes.erase(meshIt, m_loadingMeshes.end());

	if (m_loadingTextures.empty() && m_loadingMeshes.empty())
	{
		uploadBuffers.clear();
	}
}

void Graphics::ResourceManager::LoadTexture(std::string filepath)
{
	const auto device = m_renderContext->m_graphicsDevice;
	const auto commandContext = m_renderContext->m_copyCommandContext;
	commandContext->Reset();
	const auto commandList = m_renderContext->m_copyCommandContext->GetCommandList();
	const auto PSOManager = m_renderContext->m_PSOManager;
	const auto descriptorHeapManager = m_renderContext->m_descriptorHeapManager;

	// 파일경로를 통해 텍스처를 로드합니다.
	WIC_FLAGS wicFlags = WIC_FLAGS_NONE;
	TexMetadata metadata = {};
	ScratchImage data;
	ThrowIfFailed(DirectX::LoadFromWICFile(
		std::wstring(filepath.begin(), filepath.end()).c_str(),
		wicFlags, &metadata, data, nullptr));

	/// 해셔와 관련된 내용은 
	/// 전용 클래스로 대체될 것이기 때문에 
	/// 굳이 임시변수로 만들어둔다.
	std::shared_ptr<TextureResource> textureResource
		= std::make_shared<TextureResource>(filepath,
			GOE::FileManager::GetHash(filepath));

	ComPtr<ID3D12Resource> textureDefault = nullptr;
	ComPtr<ID3D12Resource> textureUpload = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = {};
	D3D12_GPU_DESCRIPTOR_HANDLE gpuSrvHandle = {};

	// ScratchImage로부터 얻은 메타데이터로 리소스 속성을 정의합니다.
	D3D12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		static_cast<UINT64>(metadata.width),
		static_cast<UINT>(metadata.height),
		static_cast<UINT16>(metadata.arraySize),
		static_cast<UINT16>(metadata.mipLevels));

	// 디폴트 힙 생성 ---
	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(device->m_device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COMMON, // 데이터를 복사 받을 상태로 생성
		nullptr,
		IID_PPV_ARGS(&textureDefault))); // 멤버 변수 m_texture에 저장

	// 업로드 힙 생성
	// UINT64 자료형을 사용해야한다.
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(textureDefault.Get(), 0, 1);
	textureUpload = CreateUploadBuffer(nullptr, uploadBufferSize); // 빈 업로드 버퍼 생성
	CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	// ScratchImage 데이터를 GPU 리소스로 복사하도록 명령 기록 ---
	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = data.GetPixels();	// ScratchImage의 픽셀 데이터
	subresourceData.RowPitch = data.GetImage(0, 0, 0)->rowPitch; // 한 줄의 바이트 크기
	subresourceData.SlicePitch = data.GetImage(0, 0, 0)->slicePitch; // 전체 이미지의 바이트 크기

	// 내부적으로 map과 unmap을 호출합니다.
	// 커맨드 리스트에 복사 명령을 기록합니다.
	UpdateSubresources(commandList, textureDefault.Get(), textureUpload.Get(), 0, 0, 1, &subresourceData);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	/// 디스크립터힙에 SRV 생성 ---
	// 1. 디스크립터 힙으로부터 핸들 할당
	int result = descriptorHeapManager->Allocate(1, &srvHandle, &gpuSrvHandle);
	if(result == -1)
	{
		throw std::runtime_error("Failed to allocate descriptor heap for texture SRV.");
	}
	// 2. SRV 생성
	device->m_device->CreateShaderResourceView(textureDefault.Get(), &srvDesc, srvHandle);
	// 3. 커맨드제출
	commandContext->Execute();
	// 4. 리소스저장
	textureResource.get()->SetTextureDefault(std::move(textureDefault));
	// 5. 디스크립터 힙 핸들 저장
	textureResource->SetSRVHandles(srvHandle, gpuSrvHandle);

	size_t id = textureResource.get()->GetID();
	// 텍스처 리소스 맵에 추가
	m_textureResourceMap[textureResource.get()->GetID()] = std::make_shared<TextureResource>(std::move(*textureResource));
	// 로딩 중인 리소스 목록에 추가
	m_loadingTextures.push_back({ commandContext->GetCommittedFenceValue(), m_textureResourceMap[id] });
}

void Graphics::ResourceManager::CreateMeshResource(const Mesh* core_mesh)
{
	const auto commandContext = m_renderContext->m_copyCommandContext;
	m_meshResourceMap[core_mesh->GetID()] = std::make_shared<MeshResource>(
		core_mesh->GetName(),
		core_mesh->GetID());

	// 메쉬데이터를 가져옴
	Graphics::MeshData meshData(core_mesh->GetMeshData());

	// 메쉬데이터를 리소스로 변환해서 방금 추가한 메쉬리소스에 추가
	CreateVBResource(m_meshResourceMap[core_mesh->GetID()].get(), meshData);
	CreateIBResource(m_meshResourceMap[core_mesh->GetID()].get(), meshData);

	// 월드 행렬을 위한 상수 버퍼 생성 (초기값: 항등 행렬)
	// 공간 크기만큼 할당한다.
	m_meshResourceMap[core_mesh->GetID()]->SetCB(
		CreateCBResource(meshData.boneOffsets.data(), sizeof(XMFLOAT4X4) * 128));

	// 추가된 메쉬리소스에  modelID와 meshIndex를 설정한다.
	// 모델 id 도 넣어야 한다.
	m_meshResourceMap[core_mesh->GetID()]->SetMeshIndex(core_mesh->GetMeshIndex());
	m_meshResourceMap[core_mesh->GetID()]->SetModelID(core_mesh->GetModelID());

	m_loadingMeshes.push_back({ commandContext->GetCommittedFenceValue(), m_meshResourceMap[core_mesh->GetID()] });
}


/// <summary>
/// 범용 upload버퍼 생성함수
/// </summary>
/// <param name="initialData">시작 포인터</param>
/// <param name="bufferSize">데이터 크기</param>
/// <param name="initialState">상태 설정</param>
/// <returns></returns>
ComPtr<ID3D12Resource> Graphics::ResourceManager::CreateUploadBuffer(const void* initialData, size_t bufferSize)
{
	const auto device = m_renderContext->m_graphicsDevice;
	ComPtr<ID3D12Resource> uploadBuffer = nullptr;
	uploadBuffers.push_back(uploadBuffer);

	D3D12_HEAP_PROPERTIES uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	ThrowIfFailed(device->m_device->CreateCommittedResource(
		&uploadHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&uploadBuffers.back())
	));

	if (initialData)
	{
		UINT8* pDataBegin = nullptr;
		D3D12_RANGE readRange = { 0, 0 }; // CPU에서 읽지 않으므로 범위는 0입니다.
		ThrowIfFailed(uploadBuffers.back()->Map(0, &readRange, reinterpret_cast<void**>(&pDataBegin)));
		memcpy(pDataBegin, initialData, bufferSize);
		uploadBuffers.back()->Unmap(0, nullptr);
	}

	// 생성된 리소스 객체를 반환합니다.
	return uploadBuffers.back();
}

/// <summary>
/// 범용 CB생성함수
/// 메모리사이즈를 받고 CB를 생성한다.
/// 데이터가 있다면 초기데이터로 복사한다.
/// </summary>
/// <param name="initialData">초기 데이터의 메모리 주소</param>
/// <param name="bufferSize">버퍼 사이즈</param>
/// <param name="initialState">리소스 생성 상태</param>
/// <returns></returns>
ComPtr<ID3D12Resource> Graphics::ResourceManager::CreateCBResource(const void* initialData, size_t bufferSize, const D3D12_RESOURCE_STATES initialState)
{
	const auto device = m_renderContext->m_graphicsDevice;

	ComPtr<ID3D12Resource> constantBuffer = {};

	// CPU에서 자주 업데이트하는 버퍼는 UPLOAD 힙에 생성하는 것이 효율적입니다.
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	// 상수 버퍼의 크기는 반드시 256바이트의 배수여야 합니다.
	size_t alignedSize = (bufferSize + 255) & ~255;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = alignedSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// 리소스 생성
	HRESULT hr = device->m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		initialState, // D3D12_RESOURCE_STATE_GENERIC_READ가 기본값
		nullptr,
		IID_PPV_ARGS(&constantBuffer)
	);
	ThrowIfFailed(hr);

	// 초기 데이터가 있다면, 버퍼를 매핑하여 데이터를 복사합니다.
	if (initialData)
	{
		void* pMappedData = nullptr;
		D3D12_RANGE readRange = { 0, 0 }; // CPU에서 읽지 않으므로 범위는 0입니다.
		ThrowIfFailed(constantBuffer->Map(0, &readRange, &pMappedData));
		memcpy(pMappedData, initialData, bufferSize);
		constantBuffer->Unmap(0, nullptr);
	}
	// 생성된 리소스 객체를 반환합니다.
	return constantBuffer;

}

/// <summary>
/// 정점 버퍼를 생성하는 함수
/// </summary>
/// <param name="mesh_resource">메쉬정보가 저장될 자료형</param>
/// <param name="mesh_data">메쉬정보</param>
/// <param name="state">리소스 상태</param>
void Graphics::ResourceManager::CreateVBResource(MeshResource* mesh_resource, const Graphics::MeshData& mesh_data, const D3D12_RESOURCE_STATES state)
{
	const auto device = m_renderContext->m_graphicsDevice;
	const auto commandContext = m_renderContext->m_copyCommandContext;
	commandContext->Reset();
	const auto cmdList = commandContext->GetCommandList();

	UINT vertexBufferSize = static_cast<UINT>(sizeof(Graphics::Vertex) * mesh_data.vertices.size());
	ComPtr<ID3D12Resource> vertexBufferDefault = nullptr;
	ComPtr<ID3D12Resource> vertexBufferUpload = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

	// 1. 디폴트 힙 리소스 생성
	D3D12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	ThrowIfFailed(device->m_device->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON, // 일단 복사 대상으로 생성
		nullptr,
		IID_PPV_ARGS(&vertexBufferDefault)));

	// 2. 업로드 힙 리소스 생성 
	// 생성함수를 동해 데이터 복사도 진행합니다.
	vertexBufferUpload = CreateUploadBuffer(
		mesh_data.vertices.data(), // 초기 데이터 바로 복사
		vertexBufferSize);

	// 3. 업로드버퍼 -> 디폴트버퍼 복사
	cmdList->CopyResource(
		vertexBufferDefault.Get(),
		vertexBufferUpload.Get());

	//commandContext->TransitionToCommon(vertexBufferDefault.Get(), D3D12_RESOURCE_STATE_COPY_DEST);

	// 4. 정점버퍼 뷰 설정
	vertexBufferView.BufferLocation = vertexBufferDefault->GetGPUVirtualAddress();	// GPU에서 읽을 정점버퍼 시작 주소, 정점 버퍼의 GPU 가상 주소
	vertexBufferView.StrideInBytes = static_cast<UINT>(sizeof(Graphics::Vertex));		// 정점버퍼 전체 크기(바이트 단위)
	vertexBufferView.SizeInBytes = static_cast<UINT>(vertexBufferSize);	// 정점 하나당 크기(바이트 단위)

	commandContext->Execute();
	mesh_resource->SetVBSize(vertexBufferSize); // 정점 버퍼 크기 설정
	mesh_resource->SetVBDefault(vertexBufferDefault.Get()); // 디폴트 힙 리소스 설정
	mesh_resource->SetVBView(vertexBufferView); // 정점 버퍼 뷰 설정
}

/// <summary>
/// 인덱스 버퍼를 생성하는 함수
/// </summary>
/// <param name="mesh_resource">메쉬정보가 저장될 자료형</param>
/// <param name="mesh_data">메쉬정보</param>
/// <param name="state">리소스 상태</param>
void Graphics::ResourceManager::CreateIBResource(MeshResource* mesh_resource, const Graphics::MeshData& mesh_data, const D3D12_RESOURCE_STATES state)
{
	const auto device = m_renderContext->m_graphicsDevice;
	const auto commandContext = m_renderContext->m_copyCommandContext;
	commandContext->Reset();
	const auto cmdList = commandContext->GetCommandList();

	UINT indexBufferSize = static_cast<UINT>(sizeof(UINT) * mesh_data.indices.size());
	ComPtr<ID3D12Resource> indexBufferDefault = nullptr;
	ComPtr<ID3D12Resource> indexBufferUpload = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};

	// 1. Default Heap (GPU)
	D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
	ThrowIfFailed(device->m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&indexBufferDefault)
	));

	// 2. Upload Heap (CPU)
	indexBufferUpload = CreateUploadBuffer(
		mesh_data.indices.data(), // 초기 데이터 바로 복사
		indexBufferSize);

	// 3. 업로드버퍼 -> 디폴트버퍼 복사
	cmdList->CopyResource(
		indexBufferDefault.Get(),
		indexBufferUpload.Get());

	//commandContext->TransitionToCommon(indexBufferDefault.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
	// 6. 인덱스버퍼 뷰 생성
	indexBufferView.BufferLocation = indexBufferDefault->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = indexBufferSize;

	commandContext->Execute();
	mesh_resource->SetIndexCount(static_cast<UINT>(mesh_data.indices.size()));
	mesh_resource->SetIBSize(indexBufferSize); // 인덱스 버퍼 크기 설정
	mesh_resource->SetIBDefault(indexBufferDefault.Get()); // 디폴트 힙 리소스 설정
	mesh_resource->SetIBView(indexBufferView); // 인덱스 버퍼 뷰 설정
}

