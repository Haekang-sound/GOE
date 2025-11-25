#include "Renderer_pch.h"
#include "SwapChain.h"
#include <d3dx12/d3dx12.h>

Graphics::SwapChain::SwapChain(const HWND hWnd, const UINT frameBufferCount)
{
	m_hWnd = hWnd;
	//m_frameBufferCount = frameBufferCount;
	RECT rect = {};
	GetClientRect(m_hWnd, &rect);
	m_width = rect.right - rect.left;
	m_height = rect.bottom - rect.top;
}

Graphics::SwapChain::~SwapChain() = default;

void Graphics::SwapChain::Initialize(RenderContext* renderContext)
{
	m_renderContext = renderContext;
	SetViewport();
	CreateSwapChain();
	CreateRTVHeap();
	CreateRenderTargets();
	CreateDepthStencilBuffer();
}
void Graphics::SwapChain::CreateSwapChain()
{
	const auto device = m_renderContext->m_graphicsDevice;
	// DXGI_SWAP_CHAIN_DESC1
	// : 스왑체인의 속성을 정의하는 구조체입니다.
	// BufferCount	: 스왑체인에 포함될 백버퍼의 수
	// Format		: 백버퍼의 픽셀 형식
	// BufferUsage	: 백버퍼의 사용 용도 // 여러 용도 조합 가능(예: SHADER_INPUT | RENDER_TARGET_OUTPUT)
	// Stereo		: 스테레오 렌더링 여부
	//					TRUE면 스테레오 3D(입체 화면, VR) 지원
	//					일반게임이면 false
	// SampleDesc	: 멀티샘플링(MSAA) 옵션
	// SwapEffect	: 스왑체인의 효과를 지정합니다.	
	// Scaling		: 윈도우 크기와 스왑체인 해상도가 다를 때 화면을 어떻게 맞출지
	// SwapEffect	: 스왑(버퍼 전환) 방식
	// AlphaMode	: 알파 블렌딩 모드 지원방식
	// Flags		: 디버그, 풀스크린 전환 등 특수 옵션 지정
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = m_frameBufferCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	ComPtr<IDXGISwapChain1> swapChain;

	//  CreateSwapChainForHwnd()
	// : 이 메서드는 HWND에 대한 스왑체인을 생성합니다.
	ThrowIfFailed(device->m_dxgiFactory->CreateSwapChainForHwnd(
		device->m_renderCmdQueue.Get(),
		m_hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	// 전체화면	전환을 비활성화 합니다.
	// 스왑체인에 있는 옵션과 달리
	// DXGI_MWA_NO_ALT_ENTER을 이용한 전체화면 비활성화는
	// [alt + Enter] 키의 입력만 차단합니다.
	// 다른 옵션을 통한 전체화면은 가능합니다.
	ThrowIfFailed(device->m_dxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER));


	// IDXGISwapChain1로 생성해서
	// 업캐스트하여 IDXGISwapChain3로 변환합니다.
	// DXGI 스왑체인 생성함수는 항상 낮은버전의 스왑체인을 반환하기때분에
	// 높은 버전의 스왑체인으로 업캐스트해서 사용합니다.
	swapChain.As(&m_swapChain);
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

/// <summary>
/// 디스크립터 힙을 생성합니다.
/// 
/// </summary>
/// <returns></returns>
void Graphics::SwapChain::CreateRTVHeap()
{
	const auto device = m_renderContext->m_graphicsDevice;
	// D3D12_DESCRIPTOR_HEAP_DESC
	// : 디스크립터 힙의 속성을 정의하는 구조체입니다.
	// NumDescriptors : 디스크립터 힙에 포함될 디스크립터의 수
	// Type : 디스크립터 힙의 유형을 지정합니다.
	// Flags : 셰이더에서 사용할 수 있는지 여부를 결정합니다.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = m_frameBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	// CreateDescriptorHeap()
	// : 디스크립터 힙을 생성하는 메서드입니다.
	ThrowIfFailed(device->m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
}

/// <summary>
/// 스왑체인의 백버퍼를 위한 랜더 타겟을 생성합니다.
/// 
/// </summary>
/// <returns></returns>
void Graphics::SwapChain::CreateRenderTargets()
{
	const auto device = m_renderContext->m_graphicsDevice;
	// GetDescriptorHandleIncrementSize() 
	// : 디스크립터 "하나"의 크기(바이트 수)를 리턴
	// 이 값은 일반적으로 올바른 양만큼 설명자 배열로 핸들을 증분하는 데 사용됩니다.
	m_rtvDescriptorSize = device->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

	// 프레임카운트 == 버퍼갯수 만큼 반복합니다.
	for (UINT i = 0; i < m_frameBufferCount; ++i)
	{
		// 스왑체인의 버퍼를 가져옵니다.
		ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
		device->m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);

		// rtvHandle을 다음 디스크립터로 이동합니다.
		// 핸들을 이동시켜주지 않으면 같은 주소에 rtv를 덮어쓰게 됩니다.
		rtvHandle.ptr += m_rtvDescriptorSize;
	}

}

void Graphics::SwapChain::CreateDepthStencilBuffer()
{
	const auto device = m_renderContext->m_graphicsDevice;

	/// 깊이 스텐실 버퍼 생성
	// 설명구조체 만들고
	D3D12_RESOURCE_DESC depthStencilDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		m_width,
		m_height,
		1, 0, 1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);

	// 
	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(device->m_device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&m_depthStencilBuffer)
	));

	// DSV 디스크립터 힙 생성
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(device->m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

	// DSV 생성
	device->m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void Graphics::SwapChain::SetViewport()
{
	GetClientRect(m_hWnd, &m_scissorRect);
	m_width = m_scissorRect.right - m_scissorRect.left;
	m_height = m_scissorRect.bottom - m_scissorRect.top;

	m_aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);

	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), m_aspectRatio, 0.01f, 10000.0f);
	XMStoreFloat4x4(&m_proj, proj);

	m_viewport = {
		0.0f,                      // TopLeftX
		0.0f,                      // TopLeftY
		static_cast<float>(m_width),   // Width
		static_cast<float>(m_height),  // Height
		0.0f,                      // MinDepth
		1.0f                       // MaxDepth
	};
}