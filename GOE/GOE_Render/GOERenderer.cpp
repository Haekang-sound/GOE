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
	m_commandContext = std::make_unique<Graphics::RenderCommandContext>();
	m_copyCommandContext = std::make_unique<Graphics::CopyCommandContext>();
	m_UIManager = std::make_unique<Graphics::UIManager>();
	m_renderContext = std::make_unique<Graphics::RenderContext>();

	// 렌더컨텍스트에 각 매니저들 연결
	m_renderContext.get()->m_resourceManager = m_resourceManager.get();
	m_renderContext.get()->m_graphicsDevice = m_graphicsDevice.get();
	m_renderContext.get()->m_swapChain = m_swapChain.get();
	m_renderContext.get()->m_PSOManager = m_PSOManager.get();
	m_renderContext.get()->m_commandContext = m_commandContext.get();
	m_renderContext.get()->m_copyCommandContext = m_copyCommandContext.get();
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

	m_camera = new Camera(m_hWnd);

	ResetCommandLists();

}

void GOERenderer::OnUpdate(double dTime)
{
	m_resourceManager.get()->UpdateResourceStates();
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
	const auto device = m_renderContext.get()->m_graphicsDevice;
	const auto commandContext = m_renderContext.get()->m_commandContext;
	device->WaitForRenderFence();
	commandContext->Reset();
	const auto commandList = m_renderContext.get()->m_commandContext->GetCommandList();
	commandList->SetPipelineState(m_PSOManager.get()->m_pipelineState.Get());
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
	const auto commandList = m_commandContext.get()->GetCommandList();
	const auto resourceManager = m_resourceManager.get();

	/// 지금은 모든 모델을 그리지만 나중에는 선택적으로 그려야한다.
	/// 공유자원이 아닌 고유자원을 기준으로 랜더오브젝트의 관한 queue를 만들어야한다.
	/// 렌더오브젝트는 메쉬단위이므로 meshresource는 해쉬맵이어야한다.
	for (const auto& renderObject : m_renderObjects)
	{
		auto meshResource = resourceManager->GetMeshResource(renderObject->GetMeshID());
		auto textureResource = resourceManager->GetTextureResource(renderObject->GetTextureID());

		if (!meshResource || meshResource.get()->GetState() != Graphics::ResourceState::READY ||
			!textureResource || textureResource.get()->GetState() != Graphics::ResourceState::READY)
		{
			continue;
		}

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &resourceManager->GetMeshResource(renderObject->GetMeshID())->GetVBView());
		commandList->IASetIndexBuffer(&resourceManager->GetMeshResource(renderObject->GetMeshID())->GetIBView());

		/// 콘스탄트 버퍼의 관한 문제는 고유자원을 기준으로 랜더할때 해결될것
		// 1. 월드/뷰/프로젝션 CBV 바인딩 (루트 파라미터 0)
		commandList->SetGraphicsRootConstantBufferView(0, renderObject->GetCB()->GetGPUVirtualAddress());

		// 2. 본 변환 CBV 바인딩 (루트 파라미터 1) - RenderObject 소유
		//    RenderObject에 GetBoneCB() 와 같은 함수가 추가되어야 합니다.
		commandList->SetGraphicsRootConstantBufferView(1, renderObject->GetCBBoneMatrix()->GetGPUVirtualAddress()); // 예시: GetBoneCB() 호출

		// 3. 본 오프셋 CBV 바인딩 (루트 파라미터 2) - MeshResource 소유
		commandList->SetGraphicsRootConstantBufferView(2, resourceManager->GetMeshResource(renderObject->GetMeshID())->GetCB()->GetGPUVirtualAddress()); // MeshResource의 CB 사용

		// 4. 텍스처 서술자 테이블 바인딩 (루트 파라미터 3)
		ID3D12DescriptorHeap* ppHeaps[] = { resourceManager->GetTextureResource(renderObject->GetTextureID())->GetTextureHeap() }; // TextureResource에서 힙 가져오기
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps); // 힙 설정

		// GPU 핸들 가져오기 (TextureResource에 GetSRVGpuHandle() 같은 함수가 있으면 더 좋습니다)
		CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle(resourceManager->GetTextureResource(renderObject->GetTextureID())->GetTextureHeap()->GetGPUDescriptorHandleForHeapStart());
		// 루트 파라미터 인덱스를 3으로 변경!
		commandList->SetGraphicsRootDescriptorTable(3, srvGpuHandle); // <--- 인덱스 3 사용

		// 7. 그리기 명령
		commandList->DrawIndexedInstanced(resourceManager->GetMeshResource(renderObject->GetMeshID())->GetIndexCount(), 1, 0, 0, 0);
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

	m_swapChain.get()->m_swapChain->Present(1, 0);

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
	delete m_camera;
}

void GOERenderer::ResetCommandLists()
{
	const auto commandConetext = m_renderContext.get()->m_commandContext;
	const auto copyCommandContext = m_renderContext.get()->m_copyCommandContext;
}

void GOERenderer::FlushCommandQueue()
{
}

void GOERenderer::WaitForGPU()
{
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
	const auto resourceManager = m_resourceManager.get();
	auto newRendrObj = std::make_unique<RenderObject>(data);
	m_renderObjects.emplace_back(std::move(newRendrObj));
	// 콘스탄트버퍼를 개별적으로 생성해준다.
	// 데이터없이 적당한 크기로만 생성해도된다.
	// 이부분은 랜더시스템을 개선하면 사라지게될것이다.
	m_renderObjects.back()->SetCB(resourceManager->CreateCBResource(nullptr, sizeof(XMFLOAT4X4)));
	m_renderObjects.back()->SetCBBoneMatrix(resourceManager->CreateCBResource(m_renderObjects.back().get()->GetBoneTMBegin(), sizeof(XMFLOAT4X4) * 128));
}

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
