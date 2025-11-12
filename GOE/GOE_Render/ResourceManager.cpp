#include "Renderer_pch.h"
#include "ResourceManager.h"

#include <d3dx12/d3dx12.h>
#include "DirectXTex.h"	

#include "CommandContext.h"
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

void Graphics::ResourceManager::LoadTexture(std::string filepath, CommandContext& commandContext)
{
	const auto commandAllocator = m_renderContext->m_commandContext->m_commandAllocator;
	const auto commandList = m_renderContext->m_commandContext->m_commandList;
	const auto PSOManager = m_renderContext->m_PSOManager;
	const auto device = m_renderContext->m_graphicsDevice;

	// 파일경로를 통해 텍스처를 로드합니다.
	WIC_FLAGS wicFlags = WIC_FLAGS_NONE;
	TexMetadata metadata = {};
	ScratchImage data;

	HRESULT hr = DirectX::LoadFromWICFile(
		std::wstring(filepath.begin(), filepath.end()).c_str(),
		wicFlags, &metadata, data, nullptr);

	ThrowIfFailed(hr);

	/// 해셔와 관련된 내용은 
	/// 전용 클래스로 대체될 것이기 때문에 
	/// 굳이 임시변수로 만들어둔다.
	std::unique_ptr<TextureResource> textureResource
		= std::make_unique<TextureResource>(filepath,
			GOE::FileManager::GetHash(filepath));

	ComPtr<ID3D12Resource> textureDefault = nullptr;
	ComPtr<ID3D12Resource> textureUpload = nullptr;
	ComPtr<ID3D12DescriptorHeap> textureheap = {};
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;

	// ScratchImage로부터 얻은 메타데이터로 리소스 속성을 정의합니다.
	D3D12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		static_cast<UINT64>(metadata.width),
		static_cast<UINT>(metadata.height),
		static_cast<UINT16>(metadata.arraySize),
		static_cast<UINT16>(metadata.mipLevels));

	// --- 2. 디폴트 힙 생성 ---
	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	hr = device->m_device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, // 데이터를 복사 받을 상태로 생성
		nullptr,
		IID_PPV_ARGS(&textureDefault)); // 멤버 변수 m_texture에 저장

	if (FAILED(hr))
	{
		return;
	}

	// 3. 업로드 힙 생성
	// UINT64 자료형을 사용해야한다.
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(textureDefault.Get(), 0, 1);
	CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	hr = device->m_device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureUpload)); // 멤버 변수 m_textureUploadHeap에 저장

	if (FAILED(hr))
	{
		return;
	}

	// --- 4. ScratchImage 데이터를 GPU 리소스로 복사하도록 명령 기록 ---
	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = data.GetPixels();	// ScratchImage의 픽셀 데이터
	subresourceData.RowPitch = data.GetImage(0, 0, 0)->rowPitch; // 한 줄의 바이트 크기
	subresourceData.SlicePitch = data.GetImage(0, 0, 0)->slicePitch; // 전체 이미지의 바이트 크기


	commandAllocator->Reset();
	// 커맨드 리스트(실제 명령 기록 객체)를 리셋하고, 새 명령을 이 할당자에, 지정한 파이프라인 상태(m_pipelineState)로 기록하겠다고 선언.
	commandList->Reset(commandAllocator.Get(), PSOManager->m_pipelineState.Get());

	// 내부적으로 map과 unmap을 호출합니다.
	// 커맨드 리스트에 복사 명령을 기록합니다.
	UpdateSubresources(commandList.Get(), textureDefault.Get(), textureUpload.Get(), 0, 0, 1, &subresourceData);

	// --- 5. 텍스처 리소스 상태를 셰이더에서 읽을 수 있도록 변경 ---
	CD3DX12_RESOURCE_BARRIER barrier =
		CD3DX12_RESOURCE_BARRIER::Transition(
			textureDefault.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &barrier);

	// 디스크립터 힙 생성
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(device->m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&textureheap)));

	// --- 6. 셰이더 리소스 뷰(SRV) 생성 ---
	// SRV를 생성할 디스크립터 힙의 핸들을 가져옵니다 (m_srvHeap은 미리 생성되어 있어야 함).
	srvHandle = textureheap->GetCPUDescriptorHandleForHeapStart();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	device->m_device->CreateShaderResourceView(textureDefault.Get(), &srvDesc, srvHandle);

	textureResource.get()->SetTextureDefault(std::move(textureDefault));
	textureResource.get()->SetTextureUpload(std::move(textureUpload));
	textureResource.get()->SetTextureHeap(std::move(textureheap));
	textureResource.get()->SetSRVHandle(srvHandle);
	m_textureResourceMap[textureResource.get()->GetID()] = textureResource.get();
	m_textureResources.push_back(std::move(textureResource));


	// --- 7. 커맨드 리스트 실행 및 동기화 ---
	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	device->m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// GPU 작업이 완료될 때까지 기다립니다 (Fence 사용).
	device->m_fenceValue++;
	device->SignalFence(device->m_fenceValue);
}
