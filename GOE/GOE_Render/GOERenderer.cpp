#include "Renderer_pch.h"
#include "GOERenderer.h"

#include <d3dx12/d3dx12.h>

// 메인 그래픽스 관리자
#include "GraphicsDevice.h"
#include "SwapChain.h"
#include "PSOManager.h"
#include "CommandContext.h"
#include "UIManager.h"
#include "ResourceManager.h"

/// 카메라도 여기있으면안됨
#include "Camera.h" 

// 리소스자료형
#include "MeshResource.h"
#include "TextureResource.h"
#include "RenderObject.h"

#if defined(_DEBUG)
#include "PIX.h"
#endif

/// <summary>
/// GOERenderer의 생성자
/// 랜더러에 필요한 클래스들을 생성한다.
/// 
/// </summary>
/// <param name="hWnd">윈도우 핸들</param>
GOERenderer::GOERenderer(const HWND hWnd)
	: m_hWnd(hWnd), m_camera(nullptr)
{
	/// 분리했으니 제대로 생성해야한다.
	m_graphicsDevice = std::make_unique<Graphics::GraphicsDevice>();
	m_swapChain = std::make_unique<Graphics::SwapChain>(m_hWnd, 2);
	m_PSOManager = std::make_unique<Graphics::PSOManager>();
	m_resourceManager = std::make_unique<Graphics::ResourceManager>();
	m_commandContext = std::make_unique<Graphics::CommandContext>();
	m_UIManager = std::make_unique<Graphics::UIManager>();
	m_renderContext = std::make_unique<Graphics::RenderContext>();

	// 렌더컨텍스트에 각 매니저들 연결
	m_renderContext.get()->m_resourceManager = m_resourceManager.get();
	m_renderContext.get()->m_graphicsDevice = m_graphicsDevice.get();
	m_renderContext.get()->m_swapChain = m_swapChain.get();
	m_renderContext.get()->m_PSOManager = m_PSOManager.get();
	m_renderContext.get()->m_commandContext = m_commandContext.get();
	m_renderContext.get()->m_UIManager = m_UIManager.get();
}

/// <summary>
/// GOERenderer의 소멸자
/// 
/// </summary>
GOERenderer::~GOERenderer()
{
	OnDestroy();
}

/// <summary>
/// 초기화 함수
///
/// </summary>
void GOERenderer::OnInit()
{
	/// 순서는 중요합니다.
	m_graphicsDevice.get()->Initialize(false, true);
#if defined(_DEBUG)
	GetLatestWinPixGpuCapturerPath_Cpp17();
	// Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
	// This may happen if the application is launched through the PIX UI. 
	if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
	{
		LoadLibrary(L"C:\\Program Files\\Microsoft PIX\\2509.25\\WinPixGpuCapturer.dll");
	}
#endif
	m_swapChain.get()->Initialize(m_renderContext.get());
	m_PSOManager.get()->Initialize(m_renderContext.get());
	m_commandContext.get()->Initialize(m_renderContext.get());
	m_UIManager.get()->Initialize(m_renderContext.get());
	m_resourceManager.get()->Initialize(m_renderContext.get());

	m_camera = new Camera(m_hWnd);
}

void GOERenderer::OnUpdate(double dTime)
{
	m_camera->OnUpdate();

	// 랜더오브젝트들을 그리는구간
	// 여긴 콘스탄트 버퍼를 업데이트 하는거임
	for (const auto& renderObject : m_renderObjects)
	{
		Graphics::CB cbData = {};
		XMMATRIX world = XMLoadFloat4x4(&renderObject.get()->GetLocalTM().matrix);
		XMMATRIX vp =
			m_camera->GetViewTransform()
			* XMLoadFloat4x4(&m_swapChain.get()->m_proj);
		cbData.cameraPosition = m_camera->GetPosition();

		XMStoreFloat4x4(&cbData.world, world);
		XMStoreFloat4x4(&cbData.viewProjection, vp);

		void* pData = nullptr;
		D3D12_RANGE readRange = { 0, 0 };
		ThrowIfFailed(renderObject->GetCB()->Map(0, &readRange, &pData));
		memcpy(pData, &cbData, sizeof(Graphics::CB));
		renderObject->GetCB()->Unmap(0, nullptr);

		/// boneMatrix에 본순서대로 업데이트된 메트릭스를 채우넣으면 된다.
		XMFLOAT4X4 boneMatrix[128] = {};
		for (int i = 0; i < 128; ++i)
		{
			boneMatrix[i] = renderObject->GetBoneTM(i).matrix;
		}
		void* pBoneData = nullptr;
		D3D12_RANGE boneRange = { 0, 0 };
		ThrowIfFailed(renderObject->GetCBBoneMatrix()->Map(0, &boneRange, &pBoneData));
		memcpy(pBoneData, &boneMatrix, sizeof(XMFLOAT4X4) * 128);
		renderObject->GetCBBoneMatrix()->Unmap(0, nullptr);
	}
}

/// <summary>
/// 그리기 전에 필요한 동작을 합니다.
/// 
/// </summary>
void GOERenderer::BeginRender()
{
	const auto commandAllocator = m_renderContext.get()->m_commandContext->m_commandAllocator;
	const auto commandList = m_renderContext.get()->m_commandContext->m_commandList;
	const auto device = m_renderContext.get()->m_graphicsDevice;
	device->WaitForFence(device->m_fenceValue);

	// 1. 커맨드 할당자와 커맨드 리스트 초기화
	// 이전에 기록된 GPU 작업(커맨드 리스트)이 끝났으니, 새롭게 명령을 기록할 수 있도록 할당자(Allocator)를 리셋합니다.
	commandAllocator->Reset();
	// 커맨드 리스트(실제 명령 기록 객체)를 리셋하고, 새 명령을 이 할당자에, 지정한 파이프라인 상태(m_pipelineState)로 기록하겠다고 선언.
	commandList->Reset(commandAllocator.Get(), m_PSOManager.get()->m_pipelineState.Get());

	// 2. 그래픽스 파이프라인 세팅
		// 셰이더들이 쓸 수 있는 리소스(텍스처, 버퍼 등)들의 묶음인 Root Signature를 바인딩.
	commandList->SetGraphicsRootSignature(m_PSOManager.get()->m_rootSignature.Get());
	// 뷰포트(화면에 그릴 영역의 크기와 위치, 카메라 뷰)를 지정.
	commandList->RSSetViewports(1, &m_swapChain.get()->m_viewport);
	// ScissorRECT 지정. 이 영역 바깥은 렌더링 안 함(클리핑).
	commandList->RSSetScissorRects(1, &m_swapChain.get()->m_scissorRect);

	/*1. 베리어(Barrier)란 ?
		GPU 리소스(버퍼, 텍스처 등)의 “상태 전환”을 명시적으로 선언하는 명령
		D3D12에서 리소스는 “읽기”, “쓰기”, “카피”, “표시(Present)”, “렌더타겟”, “셰이더리소스” 등 다양한 상태를 가짐
		GPU 파이프라인의 단계마다 리소스가 “올바른 상태”에 있어야만 GPU가 올바르게 처리함
		베리어는 “지금부터 이 리소스 상태를 바꾼다”를 GPU에 알려주는 명령어*/
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_swapChain.get()->m_renderTargets[m_swapChain.get()->m_frameIndex].Get(),    // pResource
		D3D12_RESOURCE_STATE_PRESENT,           // StateBefore
		D3D12_RESOURCE_STATE_RENDER_TARGET      // StateAfter
	);

	//3. 리소스 배리어(상태 변경) – “Present → RenderTarget”
		// 현재 그릴 렌더타겟(BackBuffer)의 상태를 “화면에 표시(PRESENT)” → “렌더링(RTT)” 상태로 전환
	commandList->ResourceBarrier(1, &barrier);

	/// 4. 렌더 타겟 뷰 바인딩 + DSV 핸들 추가 
	/// 드라마틱 한 변화이기 때문에 반드시 정리하자 
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapChain.get()->m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += static_cast<unsigned __int64>(m_swapChain.get()->m_frameIndex * m_swapChain.get()->m_rtvDescriptorSize);

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_swapChain.get()->m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

	// Output Merger(최종 출력단)에 "이 렌더타겟에 그려라" 지정. 
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle); // DSV 핸들 추가

	// 5. 렌더 타겟 클리어(색상 초기화)
	const float clearColor[] = { .7f, .7f, .5f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // 깊이 버퍼 클리어
}

/// <summary>
/// 랜더링 루프를 담당합니다.
/// 
/// 이 함수는 커맨드 리스트를 채우고,
/// 커맨드 큐를 통해 실행하며,
/// 스왑체인을 통해 화면에 출력합니다.
/// </summary>
void GOERenderer::OnRender()
{
	const auto commandList = m_commandContext.get()->m_commandList;
	const auto resourceManager = m_resourceManager.get();

	/// 지금은 모든 모델을 그리지만 나중에는 선택적으로 그려야한다.
	/// 공유자원이 아닌 고유자원을 기준으로 랜더오브젝트의 관한 queue를 만들어야한다.
	/// 렌더오브젝트는 메쉬단위이므로 meshresource는 해쉬맵이어야한다.
	for (const auto& renderObject : m_renderObjects)
	{
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &m_meshResourceMap[renderObject->GetMeshID()]->GetVBView());
		commandList->IASetIndexBuffer(&m_meshResourceMap[renderObject->GetMeshID()]->GetIBView());

		/// 콘스탄트 버퍼의 관한 문제는 고유자원을 기준으로 랜더할때 해결될것
		// 1. 월드/뷰/프로젝션 CBV 바인딩 (루트 파라미터 0)
		commandList->SetGraphicsRootConstantBufferView(0, renderObject->GetCB()->GetGPUVirtualAddress());

		// 2. 본 변환 CBV 바인딩 (루트 파라미터 1) - RenderObject 소유
		//    RenderObject에 GetBoneCB() 와 같은 함수가 추가되어야 합니다.
		commandList->SetGraphicsRootConstantBufferView(1, renderObject->GetCBBoneMatrix()->GetGPUVirtualAddress()); // 예시: GetBoneCB() 호출

		// 3. 본 오프셋 CBV 바인딩 (루트 파라미터 2) - MeshResource 소유
		commandList->SetGraphicsRootConstantBufferView(2, m_meshResourceMap[renderObject->GetMeshID()]->GetCB()->GetGPUVirtualAddress()); // MeshResource의 CB 사용

		// 4. 텍스처 서술자 테이블 바인딩 (루트 파라미터 3)
		ID3D12DescriptorHeap* ppHeaps[] = {resourceManager->m_textureResourceMap[renderObject->GetTextureID()]->GetTextureHeap() }; // TextureResource에서 힙 가져오기
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps); // 힙 설정

		// GPU 핸들 가져오기 (TextureResource에 GetSRVGpuHandle() 같은 함수가 있으면 더 좋습니다)
		CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle(resourceManager->m_textureResourceMap[renderObject->GetTextureID()]->GetTextureHeap()->GetGPUDescriptorHandleForHeapStart());
		// 루트 파라미터 인덱스를 3으로 변경!
		commandList->SetGraphicsRootDescriptorTable(3, srvGpuHandle); // <--- 인덱스 3 사용

		// 7. 그리기 명령
		commandList->DrawIndexedInstanced(m_meshResourceMap[renderObject->GetMeshID()]->GetIndexCount(), 1, 0, 0, 0);
	}
	// 6. 그리기 전 세팅
}

/// <summary>
/// 그리고 나서 필요한 동작을 합니다.
/// 
/// </summary>
void GOERenderer::EndRender()
{
	const auto commandAllocator = m_renderContext.get()->m_commandContext->m_commandAllocator;
	const auto commandList = m_renderContext.get()->m_commandContext->m_commandList;
	const auto device = m_renderContext.get()->m_graphicsDevice;

	// 8. 리소스 배리어(상태 변경) – “RenderTarget → Present”
	// 렌더링이 끝났으니, 다시 "화면에 표시(PRESENT)" 상태로 전환
	// 이 상태 변경은 GPU가 커맨드 리스트를 실행하는 동안 자동으로 처리됩니다.
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_swapChain.get()->m_renderTargets[m_swapChain.get()->m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,      // StateBefore (현재 상태)
		D3D12_RESOURCE_STATE_PRESENT             // StateAfter (목표 상태)
	);

	//3. 리소스 배리어(상태 변경) – “Present → RenderTarget”
		// 현재 그릴 렌더타겟(BackBuffer)의 상태를 “화면에 표시(PRESENT)” → “렌더링(RTT)” 상태로 전환
	commandList->ResourceBarrier(1, &barrier);

	// 9. 커맨드 리스트 닫기
		// 커맨드 리스트에 더 이상 명령을 추가하지 않겠다고 선언합니다.
		// 이 메서드를 호출한 후에는 커맨드 리스트를 실행할 수 있습니다.
	commandList->Close();

	ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
	device->m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_swapChain.get()->m_swapChain->Present(1, 0);

	device->m_fenceValue++;
	device->SignalFence(device->m_fenceValue);

	// GPU 작업이 끝났으니, swapchain에서 새로운 백버퍼 인덱스를 받아옴.
	m_swapChain.get()->m_frameIndex = m_swapChain.get()->m_swapChain->GetCurrentBackBufferIndex();

}

/// <summary>
/// 랜더러의 자원을 해제합니다.
/// 
/// </summary>
void GOERenderer::OnDestroy()
{
	const auto device = m_renderContext.get()->m_graphicsDevice;
	device->WaitForFence(device->m_fenceValue);
	CloseHandle(device->m_fenceEvent);
	delete m_camera;
}

UIInitInfo* GOERenderer::GetUIInfo()
{
	const auto UImanager = m_renderContext.get()->m_UIManager;
	return UImanager->GetUIInfo();
}

UILoopInfo* GOERenderer::GetUILoopInfo()
{
	const auto UImanager = m_renderContext.get()->m_UIManager;
	return UImanager->GetUILoopInfo();
}

/// <summary>
/// 랜더오브젝트를 생성한다.
/// </summary>
void GOERenderer::AddRenderObejct(RenderObjectData& data)
{
	auto newRendrObj = std::make_unique<RenderObject>(data);
	m_renderObjects.emplace_back(std::move(newRendrObj));
	// 콘스탄트버퍼를 개별적으로 생성해준다.
	CreateRenderObjectCBResource(m_renderObjects.back().get());
}

void GOERenderer::LoadTexture(std::string filepath)
{
	const auto resourceManager = m_resourceManager.get();
	resourceManager->LoadTexture(filepath, *m_commandContext.get());
}


/// <summary>
/// 업로드 힙의 데이터를 디폴트 힙으로 복사합니다.
/// 
/// </summary>
void GOERenderer::CopyUploadHeapToDefault()
{
	const auto commandAllocator = m_renderContext.get()->m_commandContext->m_commandAllocator;
	const auto commandList = m_renderContext.get()->m_commandContext->m_commandList;
	const auto device = m_renderContext.get()->m_graphicsDevice;

	device->WaitForFence(device->m_fenceValue); // GPU가 이전 작업을 끝낼 때까지 기다립니다.

	// CopyBufferRegion() 메서드를 사용하여 업로드 힙의 데이터를 디폴트 힙으로 복사합니다.
	// 1. 커맨드 할당자와 커맨드 리스트 초기화
	// 이전에 기록된 GPU 작업(커맨드 리스트)이 끝났으니, 새롭게 명령을 기록할 수 있도록 할당자(Allocator)를 리셋합니다.
	commandAllocator->Reset();
	// 커맨드 리스트(실제 명령 기록 객체)를 리셋하고, 새 명령을 이 할당자에, 지정한 파이프라인 상태(m_pipelineState)로 기록하겠다고 선언.
	commandList->Reset(commandAllocator.Get(), m_PSOManager.get()->m_pipelineState.Get());

	// 모델 리소스의 메쉬 리소스를 순회하며 업로드 힙에서 디폴트 힙으로 복사합니다.
	// 큐브는 모델을 로드해서 그리는게 아니야
	for (const auto& meshResource : m_meshResourceMap)
	{
		commandList->CopyBufferRegion(
			meshResource.second->GetVBDefault(), 0,	// Dest
			meshResource.second->GetVBUpload(), 0,	// Src
			meshResource.second->GetVBSize()				// Size
		);
		// (3) 상태변환: 복사에서 VertexBuffer로 전환
		auto vsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			meshResource.second->GetVBDefault(),	// pResource
			D3D12_RESOURCE_STATE_COPY_DEST,		// StateBefore
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER // StateAfter
		);
		commandList->ResourceBarrier(1, &vsBarrier);

		// 인덱스 버퍼도 동일한 방식으로 복사합니다.
		commandList->CopyBufferRegion(
			meshResource.second->GetIBDefault(), 0,
			meshResource.second->GetIBUpload(), 0,
			meshResource.second->GetIBSize());

		// 5. 상태변환
		auto ibBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			meshResource.second->GetIBDefault(),	// pResource
			D3D12_RESOURCE_STATE_COPY_DEST,		// StateBefore
			D3D12_RESOURCE_STATE_INDEX_BUFFER	// StateAfter
		);

		commandList->ResourceBarrier(1, &ibBarrier);

	}

	commandList->Close();

	ID3D12CommandList* lists[] = { commandList.Get() };
	device->m_commandQueue->ExecuteCommandLists(1, lists);
	device->m_fenceValue++;
	device->SignalFence(device->m_fenceValue);
}

void GOERenderer::CreateAllMeshResources(const std::unordered_map<std::size_t, std::unique_ptr<Mesh>>& core_meshes)
{
	for (const auto& mesh : core_meshes)
	{
		// 메쉬리소스생성하고 추가한다.	
		m_meshResources.push_back(std::make_unique<MeshResource>(mesh.second.get()->GetName(), mesh.first));
		m_meshResourceMap[mesh.first] = m_meshResources.back().get();

		// 메쉬데이터를 가져옴
		Graphics::MeshData meshData(mesh.second.get()->GetMeshData());

		// 메쉬데이터를 리소스로 변환해서 방금 추가한 메쉬리소스에 추가
		CreateMeshResource(m_meshResourceMap[mesh.first], meshData);

		// 추가된 메쉬리소스에  modelID와 meshIndex를 설정한다.
		// 모델 id 도 넣어야 한다.
		m_meshResourceMap[mesh.first]->SetMeshIndex(mesh.second.get()->GetMeshIndex());
		m_meshResourceMap[mesh.first]->SetModelID(mesh.second.get()->GetModelID());
	}
}

void GOERenderer::CreateOneMeshResource(const Mesh* core_mesh)
{
	// 메쉬리소스생성하고 추가한다.		
	m_meshResources.push_back(std::make_unique<MeshResource>(core_mesh->GetName(), core_mesh->GetID()));

	m_meshResourceMap[core_mesh->GetID()] = m_meshResources.back().get();

	// 메쉬데이터를 가져옴
	Graphics::MeshData meshData(core_mesh->GetMeshData());

	// 메쉬데이터를 리소스로 변환해서 방금 추가한 메쉬리소스에 추가
	CreateMeshResource(m_meshResourceMap[core_mesh->GetID()], meshData);

	// 추가된 메쉬리소스에  modelID와 meshIndex를 설정한다.
	// 모델 id 도 넣어야 한다.
	m_meshResourceMap[core_mesh->GetID()]->SetMeshIndex(core_mesh->GetMeshIndex());
	m_meshResourceMap[core_mesh->GetID()]->SetModelID(core_mesh->GetModelID());
}

/// <summary>
/// 메쉬데이터를 메쉬리소스를 채워주는 함수
/// </summary>
/// <param name="mesh_resource">랜더러의 메쉬리소스</param>
/// <param name="mesh_data">메쉬를 구성하는 메쉬데이터</param>
void GOERenderer::CreateMeshResource(MeshResource* mesh_resource, Graphics::MeshData& mesh_data)
{
	CreateVBResource(mesh_resource, mesh_data, D3D12_RESOURCE_STATE_GENERIC_READ);
	CreateIBResource(mesh_resource, mesh_data, D3D12_RESOURCE_STATE_GENERIC_READ);
	CreateCBResource(mesh_resource, mesh_data, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void GOERenderer::CreateVBResource(MeshResource* mesh_resource, const Graphics::MeshData& mesh_data, const D3D12_RESOURCE_STATES& state)
{
	const auto device = m_renderContext.get()->m_graphicsDevice;
	/// 채워야 할 리소스
	UINT vertexBufferSize = static_cast<UINT>(sizeof(Graphics::Vertex) * mesh_data.vertices.size());
	ComPtr<ID3D12Resource> vertexBufferDefault = nullptr;
	ComPtr<ID3D12Resource> vertexBufferUpload = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

	// 1. 디폴트 힙 리소스 생성
	D3D12_HEAP_PROPERTIES defaultHeapProps = {};
	defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = vertexBufferSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;


	ThrowIfFailed(device->m_device->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, // 일단 복사 대상으로 생성
		nullptr,
		IID_PPV_ARGS(&vertexBufferDefault)
	));

	// D3D12_HEAP_PROPERTIES
	// : 힙의 속성을 정의하는 구조체입니다.
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD; // 리소스를 할당할 힙의 속성을 정의합니다. 

	// D3D12_RESOURCE_DESC
	// : 리소스의 속성을 정의하는 구조체입니다.
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;// 버퍼 리소스입니다.
	resDesc.Alignment = 0;								// 정렬은 0으로 설정합니다.(자동설정됨)
	resDesc.Width = vertexBufferSize;					// 버퍼라면 바이트 크기, 텍스쳐면 x축 픽셀 수
	resDesc.Height = 1;									// 텍스쳐의 높이, 버퍼일 경우 높이는 1로 설정
	resDesc.DepthOrArraySize = 1;						// 깊이 또는 배열 크기
	resDesc.MipLevels = 1;								// 밉맵레벨 수, 버퍼는1 (1이면 밉맵없음)
	resDesc.Format = DXGI_FORMAT_UNKNOWN;				// 버퍼는 포맷이 필요 없습니다.
	resDesc.SampleDesc.Count = 1;						// 샘플링은 1로 설정
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;	// 버퍼는 반드시 ROW_MAJOR로 설정합니다.
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;			// 리소스 플래그는 없습니다.

	// CreateCommittedResource()
	// : 커밋된 리소스를 생성하는 메서드입니다.
	// 이 메서드는 힙과 리소스를 동시에 생성합니다.
	/*“커밋된(Committed)” 리소스란,
		리소스를 생성할 때 힙(메모리 공간)도 자동으로 같이 만들어서
		리소스와 힙이 1:1로 매칭되는 가장 단순한 형태.*/
		//	반대되는 개념 : “Placed Resource”

	ThrowIfFailed(device->m_device->CreateCommittedResource(
		&heapProps,				// 힙 속성
		D3D12_HEAP_FLAG_NONE,	// 힙 플래그, 일반적으로 D3D12_HEAP_FLAG_NONE 사용
		&resDesc,				// 리소스 설명
		state, // 리소스 생성 직후의 상태
		nullptr,				// 최적화된 클리어 값 포인터(텍스처, RTV, DSV 등만 해당) 일반 버퍼는 nullptr
		IID_PPV_ARGS(&vertexBufferUpload))); // 반환될 인터페이스의 ID
	mesh_resource->SetVBUpload(vertexBufferUpload.Get()); // 업로드 힙 리소스 설정
	// 업로드 힙에 정점 데이터를 복사하기 위해
	// 업로드 힙의 시작 주소를 가져옵니다.
	UINT8* pVertexDataBegin = nullptr;

	// D3D12_RANGE
	// : 업로드 힙의 데이터를 CPU가 읽을 수 있도록 매핑할 때 사용하는 구조체입니다.
	// Map() 호출 시 : CPU가 실제로 "읽을 범위"를 지정(읽을 게 없다면 {0, 0}로 설정)
	// Unmap() 호출 시:CPU가 실제로 "썼던 범위"를 지정
	D3D12_RANGE readRange = { 0,0 }; // 읽을 필요 없는 경우(주로 데이터 쓸 때)
	/*readRange가{ 0, 0 }이면
		GPU 드라이버는
		→ 메모리 캐시에서 버퍼 내용을 "CPU 쪽으로 읽어올 필요 없다"고 판단!
		→ memcpy로 쓰기만 할 테니 “최소한의 작업”만 해줌*/

		// Map() 메서드는 업로드 힙의 데이터를 CPU가 읽을 수 있도록 매핑합니다.
	ThrowIfFailed(vertexBufferUpload->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	// memcpy() 함수를 사용하여 정점 데이터를 업로드 힙에 복사합니다.
	memcpy(pVertexDataBegin, mesh_data.vertices.data(), vertexBufferSize);
	// Unmap() 메서드는 업로드 힙의 매핑을 해제합니다.
	vertexBufferUpload->Unmap(0, nullptr);

	// D3D12_VERTEX_BUFFER_VIEW
	// : 정점 버퍼 뷰를 정의하는 구조체입니다.
	// 이후 DrawCall 시 이 정보를 넘김
	// 이 뷰는 GPU가 정점 데이터를 읽을 때 사용됩니다.
	vertexBufferView.BufferLocation = vertexBufferDefault->GetGPUVirtualAddress();	// GPU에서 읽을 정점버퍼 시작 주소, 정점 버퍼의 GPU 가상 주소
	vertexBufferView.StrideInBytes = static_cast<UINT>(sizeof(Graphics::Vertex));		// 정점버퍼 전체 크기(바이트 단위)
	vertexBufferView.SizeInBytes = static_cast<UINT>(vertexBufferSize);	// 정점 하나당 크기(바이트 단위)

	mesh_resource->SetVBSize(vertexBufferSize); // 정점 버퍼 크기 설정
	mesh_resource->SetVBDefault(vertexBufferDefault.Get()); // 디폴트 힙 리소스 설정
	mesh_resource->SetVBUpload(vertexBufferUpload.Get()); // 업로드 힙 리소스 설정
	mesh_resource->SetVBView(vertexBufferView); // 정점 버퍼 뷰 설정
}

void GOERenderer::CreateIBResource(MeshResource* mesh_resource, const Graphics::MeshData& mesh_data, const D3D12_RESOURCE_STATES& state)
{
	const auto device = m_renderContext.get()->m_graphicsDevice;
	UINT indexBufferSize = static_cast<UINT>(sizeof(UINT) * mesh_data.indices.size());
	ComPtr<ID3D12Resource> indexBufferDefault = nullptr;
	ComPtr<ID3D12Resource> indexBufferUpload = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};

	// 1. Default Heap (GPU)
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = indexBufferSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(device->m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&indexBufferDefault)
	));

	// 2. Upload Heap (CPU)
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	ThrowIfFailed(device->m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		state,
		nullptr,
		IID_PPV_ARGS(&indexBufferUpload)
	));

	// 3. 데이터 복사
	UINT8* pIndexDataBegin = nullptr;
	D3D12_RANGE readRange = { 0, 0 };
	ThrowIfFailed(indexBufferUpload->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));

	memcpy(pIndexDataBegin, mesh_data.indices.data(), indexBufferSize);
	indexBufferUpload->Unmap(0, nullptr);

	// 6. 인덱스버퍼 뷰 생성
	indexBufferView.BufferLocation = indexBufferDefault->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = indexBufferSize;

	mesh_resource->SetIndexCount(static_cast<UINT>(mesh_data.indices.size()));
	mesh_resource->SetIBSize(indexBufferSize); // 인덱스 버퍼 크기 설정
	mesh_resource->SetIBDefault(indexBufferDefault.Get()); // 디폴트 힙 리소스 설정
	mesh_resource->SetIBUpload(indexBufferUpload.Get()); // 업로드 힙 리소스 설정
	mesh_resource->SetIBView(indexBufferView); // 인덱스 버퍼 뷰 설정
}

void GOERenderer::CreateCBResource(MeshResource* mesh_resource, const Graphics::MeshData& mesh_data, const D3D12_RESOURCE_STATES& state)
{
	const auto device = m_renderContext.get()->m_graphicsDevice;

	/// 여기부턴 본매트릭스를 위한 cb생성구간이다.
	ComPtr<ID3D12Resource> constantBuffer = {};
	ComPtr<ID3D12DescriptorHeap> CBVHeap = {};
	D3D12_CPU_DESCRIPTOR_HANDLE CBVHandle = {};

	// CBV디스크립터힙 heapProps
	D3D12_HEAP_PROPERTIES matrixheapProps = {};
	matrixheapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	matrixheapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	matrixheapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	matrixheapProps.CreationNodeMask = 0;
	matrixheapProps.VisibleNodeMask = 0;

	// 리소스 description
	D3D12_RESOURCE_DESC MatrixcbDesc = {};
	MatrixcbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	MatrixcbDesc.Alignment = 0;
	MatrixcbDesc.Width = sizeof(XMFLOAT4X4) * 128; // 최소 256바이트(행렬 64 + 패딩)
	MatrixcbDesc.Height = 1;
	MatrixcbDesc.DepthOrArraySize = 1;
	MatrixcbDesc.MipLevels = 1;
	MatrixcbDesc.Format = DXGI_FORMAT_UNKNOWN;
	MatrixcbDesc.SampleDesc.Count = 1;
	MatrixcbDesc.SampleDesc.Quality = 0;
	MatrixcbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	MatrixcbDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// CB 리소스생성
	HRESULT hr = device->m_device->CreateCommittedResource(
		&matrixheapProps,
		D3D12_HEAP_FLAG_NONE,
		&MatrixcbDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constantBuffer)
	);
	ThrowIfFailed(hr);

	// CBV 디스크립터 힙 생성
	D3D12_DESCRIPTOR_HEAP_DESC MatrixheapDescCBV = {};
	MatrixheapDescCBV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // Constant Buffer View, Shader Resource View, Unordered Access View
	MatrixheapDescCBV.NumDescriptors = 1; // CBV 하나만 사용
	MatrixheapDescCBV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // 셰이더에서 접근 가능하도록 설정
	MatrixheapDescCBV.NodeMask = 0; // 멀티 GPU 시스템에서 사용할 노드 마스크, 단일 GPU 시스템에서는 1로 설정
	device->m_device->CreateDescriptorHeap(&MatrixheapDescCBV, IID_PPV_ARGS(&CBVHeap));

	D3D12_CONSTANT_BUFFER_VIEW_DESC MatrixCBVDesc = {};
	// CBV 디스크립터 생성
	MatrixCBVDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress(); // CB 리소스의 GPU 가상 주소
	MatrixCBVDesc.SizeInBytes = ((sizeof(XMFLOAT4X4) * 128) + 255) & ~255; // CBV는 256바이트 정렬이 필요하므로, 크기를 256바이트로 올림 처리

	CBVHandle = CBVHeap->GetCPUDescriptorHandleForHeapStart();
	device->m_device->CreateConstantBufferView(&MatrixCBVDesc, CBVHandle);

	/// 여기 잘 채우면 될듯? 
	/// 이미 float4x4를 채워놨으니까 안해도될듯
	XMFLOAT4X4 boneMatrix[128] = {};
	// 1. 실제 boneOffset 데이터를 복사합니다.
	// vector의 data() 포인터를 사용해 메모리를 직접 복사하는 것이 효율적입니다.
	size_t offsetCount = mesh_data.boneOffsets.size();
	for (int i = 0; i < offsetCount; ++i)
	{
		boneMatrix[i] = mesh_data.boneOffsets[i];
	}
	for (size_t i = mesh_data.boneOffsets.size(); i < 128; ++i)
	{
		DirectX::XMStoreFloat4x4(&boneMatrix[i], /*XMMatrixTranspose(*/DirectX::XMMatrixIdentity());
	}

	void* pBoneData = nullptr;
	D3D12_RANGE boneReadRange = { 0, 0 };
	ThrowIfFailed(constantBuffer->Map(0, &boneReadRange, &pBoneData));
	memcpy(pBoneData, boneMatrix, sizeof(boneMatrix));
	constantBuffer->Unmap(0, nullptr);

	mesh_resource->SetCB(constantBuffer.Get()); // 업로드 힙 리소스 설정
	mesh_resource->SetCBVHeap(CBVHeap.Get()); // Constant Buffer View 디스크립터 힙 설정
	mesh_resource->SetCBVHandle(std::move(CBVHandle)); // Constant Buffer View 디스크립터 핸들 설정

}

void GOERenderer::CreateRenderObjectCBResource(RenderObject* render_object, const D3D12_RESOURCE_STATES& state)
{
	const auto device = m_renderContext.get()->m_graphicsDevice;
	ComPtr<ID3D12Resource> constantBuffer = {};
	ComPtr<ID3D12DescriptorHeap> CBVHeap = {};
	D3D12_CPU_DESCRIPTOR_HANDLE CBVHandle = {};

	// CBV디스크립터힙 heapProps
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 0;
	heapProps.VisibleNodeMask = 0;

	// 리소스 description
	D3D12_RESOURCE_DESC cbDesc = {};
	cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbDesc.Alignment = 0;
	cbDesc.Width = 256; // 최소 256바이트(행렬 64 + 패딩)
	cbDesc.Height = 1;
	cbDesc.DepthOrArraySize = 1;
	cbDesc.MipLevels = 1;
	cbDesc.Format = DXGI_FORMAT_UNKNOWN;
	cbDesc.SampleDesc.Count = 1;
	cbDesc.SampleDesc.Quality = 0;
	cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	cbDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// CB 리소스생성
	HRESULT hr = device->m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&cbDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constantBuffer)
	);
	ThrowIfFailed(hr);

	// CBV 디스크립터 힙 생성
	D3D12_DESCRIPTOR_HEAP_DESC heapDescCBV = {};
	heapDescCBV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // Constant Buffer View, Shader Resource View, Unordered Access View
	heapDescCBV.NumDescriptors = 1; // CBV 하나만 사용
	heapDescCBV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // 셰이더에서 접근 가능하도록 설정
	heapDescCBV.NodeMask = 0; // 멀티 GPU 시스템에서 사용할 노드 마스크, 단일 GPU 시스템에서는 1로 설정
	device->m_device->CreateDescriptorHeap(&heapDescCBV, IID_PPV_ARGS(&CBVHeap));

	D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc = {};
	// CBV 디스크립터 생성
	CBVDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress(); // CB 리소스의 GPU 가상 주소
	CBVDesc.SizeInBytes = (sizeof(Graphics::CB) + 255) & ~255; // CBV는 256바이트 정렬이 필요하므로, 크기를 256바이트로 올림 처리

	CBVHandle = CBVHeap->GetCPUDescriptorHandleForHeapStart();
	device->m_device->CreateConstantBufferView(&CBVDesc, CBVHandle);

	Graphics::CB cbData = {};

	DirectX::XMStoreFloat4x4(&cbData.world, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&cbData.viewProjection, DirectX::XMMatrixIdentity());

	void* pData = nullptr;
	D3D12_RANGE readRange = { 0, 0 };
	ThrowIfFailed(constantBuffer->Map(0, &readRange, &pData));
	memcpy(pData, &cbData, sizeof(Graphics::CB));
	constantBuffer->Unmap(0, nullptr);

	render_object->SetCB(constantBuffer.Get()); // 업로드 힙 리소스 설정
	render_object->SetCBVHeap(CBVHeap.Get()); // Constant Buffer View 디스크립터 힙 설정
	render_object->SetCBVHandle(std::move(CBVHandle)); // Constant Buffer View 디스크립터 핸들 설정

	/// 여기부턴 본매트릭스를 위한 cb생성구간이다.
	ComPtr<ID3D12Resource> boneBuffer = {};
	ComPtr<ID3D12DescriptorHeap> boneCBVHeap = {};
	D3D12_CPU_DESCRIPTOR_HANDLE boneCBVHandle = {};

	// CBV디스크립터힙 heapProps
	D3D12_HEAP_PROPERTIES matrixheapProps = {};
	matrixheapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	matrixheapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	matrixheapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	matrixheapProps.CreationNodeMask = 0;
	matrixheapProps.VisibleNodeMask = 0;

	// 리소스 description
	D3D12_RESOURCE_DESC MatrixcbDesc = {};
	MatrixcbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	MatrixcbDesc.Alignment = 0;
	MatrixcbDesc.Width = sizeof(XMFLOAT4X4) * 128; // 최소 256바이트(행렬 64 + 패딩)
	MatrixcbDesc.Height = 1;
	MatrixcbDesc.DepthOrArraySize = 1;
	MatrixcbDesc.MipLevels = 1;
	MatrixcbDesc.Format = DXGI_FORMAT_UNKNOWN;
	MatrixcbDesc.SampleDesc.Count = 1;
	MatrixcbDesc.SampleDesc.Quality = 0;
	MatrixcbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	MatrixcbDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// CB 리소스생성
	hr = device->m_device->CreateCommittedResource(
		&matrixheapProps,
		D3D12_HEAP_FLAG_NONE,
		&MatrixcbDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&boneBuffer)
	);
	ThrowIfFailed(hr);

	// CBV 디스크립터 힙 생성
	D3D12_DESCRIPTOR_HEAP_DESC MatrixheapDescCBV = {};
	MatrixheapDescCBV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // Constant Buffer View, Shader Resource View, Unordered Access View
	MatrixheapDescCBV.NumDescriptors = 1; // CBV 하나만 사용
	MatrixheapDescCBV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // 셰이더에서 접근 가능하도록 설정
	MatrixheapDescCBV.NodeMask = 0; // 멀티 GPU 시스템에서 사용할 노드 마스크, 단일 GPU 시스템에서는 1로 설정
	device->m_device->CreateDescriptorHeap(&MatrixheapDescCBV, IID_PPV_ARGS(&boneCBVHeap));

	D3D12_CONSTANT_BUFFER_VIEW_DESC MatrixCBVDesc = {};
	// CBV 디스크립터 생성
	MatrixCBVDesc.BufferLocation = boneBuffer->GetGPUVirtualAddress(); // CB 리소스의 GPU 가상 주소
	MatrixCBVDesc.SizeInBytes = ((sizeof(XMFLOAT4X4) * 128) + 255) & ~255; // CBV는 256바이트 정렬이 필요하므로, 크기를 256바이트로 올림 처리

	boneCBVHandle = boneCBVHeap->GetCPUDescriptorHandleForHeapStart();
	device->m_device->CreateConstantBufferView(&MatrixCBVDesc, boneCBVHandle);

	XMFLOAT4X4 boneMatrix[128] = {};
	for (int i = 0; i < 128; ++i)
	{
		DirectX::XMStoreFloat4x4(&boneMatrix[i], DirectX::XMMatrixIdentity());
	}

	void* pBoneData = nullptr;
	D3D12_RANGE boneReadRange = { 0, 0 };
	ThrowIfFailed(boneBuffer->Map(0, &boneReadRange, &pBoneData));
	//memcpy(pBoneData, &pBoneData, sizeof(Graphics::Matrix4x4)*128);
	// 생성해둔 boneMatrix 배열의 데이터를 복사해야 합니다.
	memcpy(pBoneData, boneMatrix, sizeof(boneMatrix));
	boneBuffer->Unmap(0, nullptr);

	render_object->SetCBBoneMatrix(boneBuffer.Get()); // 업로드 힙 리소스 설정
	render_object->SetCBVoneMatrixHeap(boneCBVHeap.Get()); // Constant Buffer View 디스크립터 힙 설정
	render_object->SetCBVoneMatrixHandle(std::move(boneCBVHandle)); // Constant Buffer View 디스크립터 핸들 설정



}