#include "GOERenderer.h"

GOERenderer::GOERenderer(HWND hWnd)
{
	m_hWnd = hWnd;
	// 클라이언트 영역의 크기를 가져옵니다.
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	m_width = rect.right - rect.left;
	m_height = rect.bottom - rect.top;

	// 윈도우의 크기를 기반으로 화면 비율을 계산합니다.
	m_aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);

	// 윈도우의 크기를 기반으로 뷰포트와 시저 직사각형을 설정합니다.
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
	m_scissorRect = CD3DX12_RECT(0, 0, m_width, m_height);
}

void GOERenderer::OnInit()
{
	LoadPipeline();
	LoadAssets();
}

void GOERenderer::OnUpdate()
{
}

void GOERenderer::OnRender()
{
	/// 시작할때 wait을 넣을것

	PopulateCommandList();

	// Execute the command list.
	// 한글 : 명령 목록을 실행합니다.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	// 한글 : 프레임을 표시합니다.
	m_swapChain->Present(1, 0);

	WaitForPreviousFrame();

	/// 끝날때 signal을 넣을것
}

void GOERenderer::OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForPreviousFrame();

	CloseHandle(m_fenceEvent);
}

void GOERenderer::LoadPipeline()
{
#if defined(_DEBUG)
	ActiveDebugLayer(true);
#endif

	CreateDXGIFactory();
	ChooseAdapter();
	CreateDevice();
	CreateCommandQueue();
	CreateSwapChain();
	CreateDescriptorHeaps();
	CreateRenderTargets();
	CreateCommandAllocator();
}

void GOERenderer::LoadAssets()
{
	CreateRootSignature();
	CompileShaders();
	CreatePipelineState();
	CreateCommandList();
	CreateVertexBuffer();
	SetVertexBufferView();
	CreateFence();
}

/// <summary>
/// 한 프레임동안 실행될 커맨드리스트를 채웁니다.
/// </summary>
void GOERenderer::PopulateCommandList()
{
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());


	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// 한글 : 백 버퍼가 렌더 타겟으로 사용될 것임을 나타냅니다.
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_commandList->ResourceBarrier(1, &barrier);


	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->DrawInstanced(3, 1, 0, 0);

	// Indicate that the back buffer will now be used to present.
	// 한글 : 백 버퍼가 이제 표시되는 데 사용될 것임을 나타냅니다.
	CD3DX12_RESOURCE_BARRIER b2arrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	m_commandList->ResourceBarrier(1, &b2arrier);


	m_commandList->Close();
}

void GOERenderer::WaitForPreviousFrame()
{

	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.
	// 한글로 번역
	// 프레임이 완료될 때까지 기다리는 것은 모범 사례가 아닙니다.
	// 이는 단순성을 위해 구현된 코드입니다. D3D12HelloFrameBuffering 샘플은
	// 효율적인 리소스 사용과 GPU 활용 극대화를 위해 펜스를 사용하는 방법을 보여줍니다.

	// Signal and increment the fence value.
	const UINT64 fence = m_fenceValue;
	m_commandQueue->Signal(m_fence.Get(), fence);
	m_fenceValue++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		m_fence->SetEventOnCompletion(fence, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void GOERenderer::GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter)
{
	*ppAdapter = nullptr;
	for (UINT adapterIndex = 0; ; ++adapterIndex)
	{
		ComPtr<IDXGIAdapter1> adapter;
		if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &adapter))
		{
			break; // 더 이상 어댑터 없음
		}

		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		// 소프트웨어(=WARP) 어댑터는 무시
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		// 실제 D3D12를 지원하는 어댑터(즉, 진짜 GPU)만 선택!
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
		{
			*ppAdapter = adapter.Detach();
			return;
		}
	}
}


HRESULT GOERenderer::CreateDXGIFactory()
{
	return 	CreateDXGIFactory2(m_dxgiFactoryFlags, IID_PPV_ARGS(&m_dxgiFactory));
	return E_NOTIMPL;
}

HRESULT GOERenderer::ActiveDebugLayer(bool isOn)
{
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();

		// Enable additional debug layers.
		// 번역 : 추가 디버그 레이어 활성화
		m_dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}

	if (isOn)
	{
		m_dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
	else
	{
		m_dxgiFactoryFlags &= ~DXGI_CREATE_FACTORY_DEBUG;
	}

	return E_NOTIMPL;
}

HRESULT GOERenderer::ChooseAdapter()
{
	if (m_useWarpDevice)
	{
		// WARP 어댑터를 사용합니다.
		// WARP는 Windows Advanced Rasterization Platform의 약자로, 
		// 소프트웨어 렌더링을 위한 Direct3D 12의 구현입니다.
		ComPtr<IDXGIAdapter> warpAdapter;
		m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
		if (warpAdapter == nullptr)
		{
			return E_FAIL; // WARP 어댑터를 찾을 수 없음
		}

	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(m_dxgiFactory.Get(), &hardwareAdapter);
		if (hardwareAdapter == nullptr)
		{
			return E_FAIL; // 하드웨어 어댑터를 찾을 수 없음
		}
	}

	return E_NOTIMPL;
}

HRESULT GOERenderer::CreateDevice()
{
	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

		D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		);
	}
	else
	{
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(m_dxgiFactory.Get(), &hardwareAdapter);

		D3D12CreateDevice(
			nullptr,
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		);
	}
	if (m_device == nullptr)
	{
		return E_FAIL;
	}
	return E_NOTIMPL;
}

HRESULT GOERenderer::CreateCommandQueue()
{
	// Describe and create the command queue.
	// 번역 : 명령 큐 설명 및 생성
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
	return E_NOTIMPL;
}

HRESULT GOERenderer::CreateSwapChain()
{
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	m_dxgiFactory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
		m_hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	);

	// 전체화면 지원하지 않음
	m_dxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	swapChain.As(&m_swapChain);
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	return E_NOTIMPL;
}

HRESULT GOERenderer::CreateDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));

	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	return E_NOTIMPL;
}

HRESULT GOERenderer::CreateRenderTargets()
{
	// Create frame resources.
	// 한글 : 프레임 리소스 생성
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		// 한글 : 각 프레임에 대한 RTV 생성
		for (UINT n = 0; n < FrameCount; n++)
		{
			m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
	}
	return E_NOTIMPL;
}

HRESULT GOERenderer::CreateCommandAllocator()
{
	m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
	return E_NOTIMPL;
}

HRESULT GOERenderer::CreateRootSignature()
{
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
	return E_NOTIMPL;
}

HRESULT GOERenderer::CompileShaders()
{
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	// 한글 : 그래픽 디버깅 도구로 셰이더 디버깅 개선
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	// Compile the shaders.
	// 한글 : 셰이더 컴파일
	D3DCompileFromFile(L"D:\\project\\GOE\\GOE\\GOERender\\shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &m_pixelShader, nullptr);
	D3DCompileFromFile(L"D:\\project\\GOE\\GOE\\GOERender\\shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &m_vertexShader, nullptr);

	// Define the vertex input layout.
	// 한글 : 정점 입력 레이아웃 정의
	D3D12_INPUT_ELEMENT_DESC b;
	b = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	m_inputElementDescs[0] = b;
	b = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	m_inputElementDescs[1] = b;
	return E_NOTIMPL;
}

HRESULT GOERenderer::CreatePipelineState()
{

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { m_inputElementDescs, _countof(m_inputElementDescs) };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
	return E_NOTIMPL;
}

HRESULT GOERenderer::CreateCommandList()
{
	m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList));
	// 명령 목록은 기록 상태로 생성되지만 아직 기록할 내용이 없습니다.
	m_commandList->Close();
	return E_NOTIMPL;
}

HRESULT GOERenderer::CreateVertexBuffer()
{
	// 삼각형의 정보
	Vertex a;
	a = { { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } };
	m_triangleVertices[0] = a;
	a = { { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } };
	m_triangleVertices[1] = a;
	a = { { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } };
	m_triangleVertices[2] = a;

	m_vertexBufferSize = sizeof(m_triangleVertices);

	// Note: using upload heaps to transfer static data like vert buffers is not 
	// recommended. Every time the GPU needs it, the upload heap will be marshalled 
	// over. Please read up on Default Heap usage. An upload heap is used here for 
	// code simplicity and because there are very few verts to actually transfer.
	// 한글 : 업로드 힙을 사용하여 정적 데이터(예: 정점 버퍼)를 전송하는 것은 권장되지 않습니다.

	// Create the vertex buffer resource on the GPU.
	// 한글 : GPU에서 정점 버퍼 리소스 생성
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize);

	m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer));
	return E_NOTIMPL;
}

HRESULT GOERenderer::SetVertexBufferView()
{
	UINT8* pVertexDataBegin;				// We do not intend to read from this resource on the CPU. 
	CD3DX12_RANGE readRange(0, 0);			// 한글 : CPU에서 이 리소스를 읽을 의도가 없습니다.
	m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
	memcpy(pVertexDataBegin, m_triangleVertices, sizeof(m_triangleVertices));
	m_vertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	// 한글 : 정점 버퍼 뷰 초기화
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_vertexBufferView.SizeInBytes = m_vertexBufferSize;
	return E_NOTIMPL;
}

HRESULT GOERenderer::CreateFence()
{
	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	// 한글 : 동기화 객체를 생성하고 자산이 GPU에 업로드될 때까지 기다립니다.
	{
		m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
		m_fenceValue = 1;

		// Create an event handle to use for frame synchronization.
		// 한글 : 프레임 동기화를 위해 사용할 이벤트 핸들 생성
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			HRESULT_FROM_WIN32(GetLastError());
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		// 한글 : 명령 목록이 실행될 때까지 기다립니다. 현재 메인 루프에서 동일한 명령 목록을 재사용하고 있지만,
		// 지금은 계속하기 전에 설정이 완료될 때까지 기다리기만 하면 됩니다.
		WaitForPreviousFrame();
	}
	return E_NOTIMPL;
}
