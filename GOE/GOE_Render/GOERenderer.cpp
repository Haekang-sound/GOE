#include "Renderer_pch.h"
#include "GOERenderer.h"

#include <d3dx12/d3dx12.h>

// 메인 그래픽스 관리자
#include "GraphicsDevice.h"
#include "SwapChain.h"
#include "PSOManager.h"
#include "UIManager.h"
#include "ResourceManager.h"
#include "CopyCommandContext.h"
#include "RenderCommandContext.h"
#include "DescriptorHeapManager.h"

// 리소스자료형
#include "MeshResource.h"
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
	: m_hWnd(hWnd)
{
	/// 분리했으니 제대로 생성해야한다.
	m_graphicsDevice = std::make_unique<Graphics::GraphicsDevice>();
	m_swapChain = std::make_unique<Graphics::SwapChain>(m_hWnd, 2);
	m_PSOManager = std::make_unique<Graphics::PSOManager>();
	m_resourceManager = std::make_unique<Graphics::ResourceManager>();
	m_commandContext = std::make_unique<Graphics::RenderCommandContext>();
	m_copyCommandContext = std::make_unique<Graphics::CopyCommandContext>();
	m_UIManager = std::make_unique<Graphics::UIManager>();
	m_descriptorHeapManager = std::make_unique<Graphics::DescriptorHeapManager>();
	m_renderContext = std::make_unique<Graphics::RenderContext>();

	// 렌더컨텍스트에 각 매니저들 연결
	m_renderContext.get()->m_resourceManager = m_resourceManager.get();
	m_renderContext.get()->m_graphicsDevice = m_graphicsDevice.get();
	m_renderContext.get()->m_swapChain = m_swapChain.get();
	m_renderContext.get()->m_PSOManager = m_PSOManager.get();
	m_renderContext.get()->m_commandContext = m_commandContext.get();
	m_renderContext.get()->m_copyCommandContext = m_copyCommandContext.get();
	m_renderContext.get()->m_UIManager = m_UIManager.get();
	m_renderContext.get()->m_descriptorHeapManager = m_descriptorHeapManager.get();
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
/// 랜더러의 필수클래스들을 생성하고 초기화 합니다.
///	0. PIX
///	1. 그래픽스 디바이스
/// 2. 스왑체인
/// 3. PSO매니저
/// 4. 커맨드 컨텍스트
/// 5. UI매니저
/// 6. 리소스 매니저
/// 
/// </summary>
void GOERenderer::OnInit()
{
#if defined(_DEBUG)
	GetLatestWinPixGpuCapturerPath_Cpp17();
	// Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
	// This may happen if the application is launched through the PIX UI. 
	if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
	{
		LoadLibrary(L"C:\\Program Files\\Microsoft PIX\\2509.25\\WinPixGpuCapturer.dll");
	}
#endif
	m_graphicsDevice.get()->Initialize(false, true);
	m_swapChain.get()->Initialize(m_renderContext.get());
	m_PSOManager.get()->Initialize(m_renderContext.get());
	m_commandContext.get()->Initialize(m_renderContext.get());
	m_copyCommandContext.get()->Initialize(m_renderContext.get());
	m_UIManager.get()->Initialize(m_renderContext.get());
	m_resourceManager.get()->Initialize(m_renderContext.get());
	m_descriptorHeapManager.get()->Initialize(m_renderContext.get());
}

void GOERenderer::OnUpdate(double dTime)
{
	m_resourceManager.get()->UpdateResourceStates();
	
	// 카메라 행렬 계산 (DirectXMath 사용)
	XMMATRIX cameraWorld = XMLoadFloat4x4(&m_cameraData.worldMatrix.matrix);
	XMMATRIX view = XMMatrixInverse(nullptr, cameraWorld);
	XMMATRIX proj = XMMatrixPerspectiveFovLH(
		m_cameraData.fov, 
		m_cameraData.aspectRatio, 
		m_cameraData.nearZ, 
		m_cameraData.farZ
	);
	XMMATRIX vp = view * proj;

	// 랜더오브젝트들을 그리는구간
	// 여긴 콘스탄트 버퍼를 업데이트 하는거임
	for (auto& renderObject : m_renderObjects)
	{
		Graphics::CB cbData = {};
		XMMATRIX world = XMLoadFloat4x4(&renderObject.GetLocalTM().matrix);
		
		cbData.cameraPosition = m_cameraData.position.vec;

		XMStoreFloat4x4(&cbData.world, world);
		XMStoreFloat4x4(&cbData.viewProjection, vp);

		/*	void* pData = nullptr;
			D3D12_RANGE readRange = { 0, 0 };
			ThrowIfFailed(renderObject.GetCB()->Map(0, &readRange, &pData));*/
			//memcpy(pData, &cbData, sizeof(Graphics::CB));
			//renderObject.GetCB()->Unmap(0, nullptr);

			/// boneMatrix에 본순서대로 업데이트된 메트릭스를 채우넣으면 된다.
		XMFLOAT4X4 boneMatrix[128] = {};
		for (int i = 0; i < 128; ++i)
		{
			boneMatrix[i] = renderObject.GetBoneTM(i).matrix;
		}
		//void* pBoneData = nullptr;
		//D3D12_RANGE boneRange = { 0, 0 };
		//ThrowIfFailed(renderObject.GetCBBoneMatrix()->Map(0, &boneRange, &pBoneData));
		//memcpy(pBoneData, &boneMatrix, sizeof(XMFLOAT4X4) * 128);
		//renderObject.GetCBBoneMatrix()->Unmap(0, nullptr);
	}
}

/// <summary>
/// 그리기 전에 필요한 동작을 합니다.
/// 
/// </summary>
void GOERenderer::BeginRender()
{
	const auto device = m_renderContext.get()->m_graphicsDevice;
	const auto commandContext = m_renderContext.get()->m_commandContext;
	device->WaitForRenderFence();
	commandContext->Reset();
	const auto commandList = m_renderContext.get()->m_commandContext->GetCommandList();
	const auto descriptorHeapManager = m_renderContext.get()->m_descriptorHeapManager;

	// 동적힙의 대한 할당초기화
	descriptorHeapManager->ResetDynamicHeap();
	descriptorHeapManager->ResetCB();

	commandList->SetPipelineState(m_PSOManager.get()->GetPipelineState().Get());
	// 2. 그래픽스 파이프라인 세팅
		// 셰이더들이 쓸 수 있는 리소스(텍스처, 버퍼 등)들의 묶음인 Root Signature를 바인딩.
	commandList->SetGraphicsRootSignature(m_PSOManager.get()->GetRootSignature().Get());
	// 뷰포트(화면에 그릴 영역의 크기와 위치, 카메라 뷰)를 지정.
	commandList->RSSetViewports(1, &m_swapChain.get()->m_viewport);
	// ScissorRECT 지정. 이 영역 바깥은 렌더링 안 함(클리핑).
	commandList->RSSetScissorRects(1, &m_swapChain.get()->m_scissorRect);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_swapChain.get()->m_renderTargets[m_swapChain.get()->m_frameIndex].Get(),    // pResource
		D3D12_RESOURCE_STATE_PRESENT,           // StateBefore
		D3D12_RESOURCE_STATE_RENDER_TARGET      // StateAfter
	);
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
	const auto device = m_renderContext.get()->m_graphicsDevice;
	const auto commandList = m_commandContext.get()->GetCommandList();
	const auto resourceManager = m_resourceManager.get();
	const auto descriptorHeapManager = m_descriptorHeapManager.get();

	ID3D12DescriptorHeap* ppHeaps[] = { descriptorHeapManager->GetDynamicSRVHeap() };
	commandList->SetDescriptorHeaps(1, ppHeaps); // 힙 설정

	// 카메라 행렬 계산 (OnRender에서도 필요하면 재계산하거나 멤버 변수에 저장해두고 사용)
	// OnUpdate에서 이미 cbData 계산 로직이 있지만, 여기서는 커맨드 리스트에 실제 명령을 내리는 곳임.
	// 상수 버퍼 할당을 위해 여기서 다시 계산.
	XMMATRIX cameraWorld = XMLoadFloat4x4(&m_cameraData.worldMatrix.matrix);
	XMMATRIX view = XMMatrixInverse(nullptr, cameraWorld);
	XMMATRIX proj = XMMatrixPerspectiveFovLH(
		m_cameraData.fov,
		m_cameraData.aspectRatio,
		m_cameraData.nearZ,
		m_cameraData.farZ
	);
	XMMATRIX vp = view * proj;

	for (auto& renderObject : m_renderObjects)
	{
		auto meshResource = resourceManager->GetMeshResource(renderObject.GetMeshID());
		auto textureResource = resourceManager->GetTextureResource(renderObject.GetTextureID());

		// 리소스 로딩여부를 확인하고 해당되지 않으면 스킵
		if (!meshResource || meshResource.get()->GetState() != Graphics::ResourceState::READY ||
			!textureResource || textureResource.get()->GetState() != Graphics::ResourceState::READY)
		{
			continue;
		}

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &meshResource->GetVBView());
		commandList->IASetIndexBuffer(&meshResource->GetIBView());

		/// 콘스탄트 버퍼의 관한 문제는 고유자원을 기준으로 랜더할때 해결될것
		// 1. 월드/뷰/프로젝션 CBV 바인딩 (루트 파라미터 0)
		Graphics::CB cbData = {};

		// RenderObject에서 데이터 가져오기 (포인터면 ->, 객체면 .)
		XMMATRIX world = XMLoadFloat4x4(&renderObject.GetLocalTM().matrix);
		
		cbData.cameraPosition = m_cameraData.position.vec;
		XMStoreFloat4x4(&cbData.world, world);
		XMStoreFloat4x4(&cbData.viewProjection, vp);

		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = descriptorHeapManager->AllocateConstantBuffer(&cbData, sizeof(Graphics::CB));
		commandList->SetGraphicsRootConstantBufferView(0, cbAddress);

		// 2. 본 변환 행렬 CBV 바인딩 (루트 파라미터 1)
		Graphics::Matrix4x4* boneData = renderObject.GetBoneTMBegin();
		size_t boneDataSize = sizeof(Graphics::Matrix4x4) * 128;
		D3D12_GPU_VIRTUAL_ADDRESS boneAddress = descriptorHeapManager->AllocateConstantBuffer(boneData, boneDataSize);
		commandList->SetGraphicsRootConstantBufferView(1, boneAddress);

		// 3. 본 오프셋 CBV 바인딩 (루트 파라미터 2) - MeshResource 소유
		commandList->SetGraphicsRootConstantBufferView(2, resourceManager->GetMeshResource(renderObject.GetMeshID())->GetCB()->GetGPUVirtualAddress()); // MeshResource의 CB 사용

		// 핸들을 받아올 GPU 디스크립터 핸들
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};
		descriptorHeapManager->CopyToDynamicHeap(
			textureResource->GetSRVCpuHandle(), // 원본 CPU 핸들
			&gpuHandle); // 동적 힙의 GPU 핸들로 복사

		// 루트 파라미터 인덱스를 3으로 변경!
		commandList->SetGraphicsRootDescriptorTable(3, gpuHandle); // <--- 인덱스 3 사용

		// 7. 그리기 명령
		commandList->DrawIndexedInstanced(meshResource->GetIndexCount(), 1, 0, 0, 0);
	}
}

/// <summary>
/// 그리고 나서 필요한 동작을 합니다.
/// 
/// </summary>
void GOERenderer::EndRender()
{
	const auto device = m_renderContext.get()->m_graphicsDevice;
	const auto commandContext = m_renderContext.get()->m_commandContext;
	const auto commandList = m_renderContext.get()->m_commandContext->GetCommandList();

	m_commandContext.get()->TransitionToPresent(
		m_swapChain.get()->m_renderTargets[m_swapChain.get()->m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	// 9. 커맨드 리스트 닫기
		// 커맨드 리스트에 더 이상 명령을 추가하지 않겠다고 선언합니다.
		// 이 메서드를 호출한 후에는 커맨드 리스트를 실행할 수 있습니다.
	commandContext->Execute();

	// Present의 첫번째 매개변수가 1 이면 수직동기화로 인한 프레임고정
	//m_swapChain.get()->m_swapChain->Present(1, 0);
	m_swapChain.get()->m_swapChain->Present(0, 0);

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
	device->WaitForRenderFence();
	// fenceEvent는 그래픽 디바이스가 소유하고 있으므로 여기서 닫아준다.
	CloseHandle(device->GetRenderFenceEvent());
	CloseHandle(device->GetCopyFenceEvent());
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

void GOERenderer::ReceiveRenderObejcts(std::vector<RenderObject>&& data)
{
	m_renderObjects = std::move(data);
}

//void GOERenderer::SetCameraData(const GOE::Matrix4x4& view, const GOE::Matrix4x4& proj, const GOE::FLoatVector3& pos)
//{
//	m_cameraData.viewMatrix = view;
//	m_cameraData.projectionMatrix = proj;
//	m_cameraData.position = pos;
//}



void GOERenderer::LoadTexture(std::string filepath)
{
	const auto resourceManager = m_resourceManager.get();
	resourceManager->LoadTexture(filepath);
}

void GOERenderer::CreateMeshResource(const Mesh* core_mesh)
{
	const auto resourceManager = m_resourceManager.get();
	resourceManager->CreateMeshResource(core_mesh);
}
