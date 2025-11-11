#include "Renderer_pch.h"
#include "GraphicsDevice.h"

Graphics::GraphicsDevice::~GraphicsDevice() = default;


void Graphics::GraphicsDevice::Initialize(const bool& useWarpDevice, const bool& debugLayerOn)
{
	m_useWarpDevice = useWarpDevice;
	// 디버그 레이어 활성화
	ActiveDebugLayer(debugLayerOn);
	// DXGI 팩토리 생성
	CreateDXGIFactory();
	// 하드웨어 어댑터 얻어오기
	const bool hardwareAdapter = GetHardwareAdapter();
	// 디바이스 생성
	CreateDevice(hardwareAdapter);
	// 커맨드 큐 생성
	CreateCommandQueue();
	// 펜스 생성
	CreateFence();
}

/// <summary>
/// 디버그 레이어를 활성 여부를 변수로 받아
/// 디버그 레이어를 활성화 합니다.
/// 
/// </summary>
/// <param name="isOn">bool</param>
/// <returns>디버그레이어 활성황 여부입니다.</returns>
void Graphics::GraphicsDevice::ActiveDebugLayer(const bool& isOn)
{
	// ID3D12Debug
	// : 이 인터페이스는 Direct3D 12 디버그 레이어를 활성화하는 데 사용됩니다.
	// 디버그 레이어는 개발자가 Direct3D 12 애플리케이션을 디버깅할 때 유용한 정보를 제공합니다.
	// ID3D12Debug 객체는 처음에만 쓰고 다음 부터는 잘 쓰이지 않기때문에 
	// ComPtr를 사용하여 관리합니다.
	ComPtr<ID3D12Debug> debugController;


	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		// EnableDebugLayer()
		// : 이 메서드는 Direct3D 12 디버그 레이어를 활성화합니다.
		debugController->EnableDebugLayer();
	}


	// DXGI_CREATE_FACTORY_DEBUG
	// : dxgi1_3.h 헤더 파일에 정의된 DXGI_CREATE_FACTORY_DEBUG 플래그를 사용하여
	// DXGI 팩토리를 생성할 때 디버그 레이어를 활성화합니다.
	if (isOn)
	{
		// 플래그 DXGI_CREATE_FACTORY_DEBUG를 활성화합니다.
		m_dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
	else
	{
		// DXGI_CREATE_FACTORY_DEBUG만 비활성화
		m_dxgiFactoryFlags &= ~DXGI_CREATE_FACTORY_DEBUG;
	}

	//	상황별 동작 정리
	//	1. EnableDebugLayer() O, DXGI_CREATE_FACTORY_DEBUG X
	//	Direct3D12 디버그 레이어만 활성화
	//	D3D12 객체(디바이스, 커맨드리스트, 리소스 등)의 오류 / 경고 메시지만 출력
	//	DXGI(어댑터, 스왑체인 등) 관련 오류는 안 나옴
	// 
	//	2. EnableDebugLayer() X, DXGI_CREATE_FACTORY_DEBUG O
	//	DXGI 디버그 레이어만 활성화
	//	DXGI 객체(팩토리, 어댑터, 스왑체인 등)의 오류 / 경고 메시지만 출력
	//	D3D12 객체 관련 에러 / 경고 메시지는 안 나옴
	// 
	//	3. 둘 다 활성화(추천)
	//	DXGI + D3D12 전체 오류 / 경고 메시지 출력
	//	그래픽스 초기화~실행 전체 단계의 이슈를 전부 빠짐없이 잡을 수 있음
	// 
	//	4. 둘 다 비활성화
	//	아무런 디버그 메시지 없음
	//	버그 찾기 매우 어려움(블랙박스 느낌)
}


/// <summary>
/// DXGI 팩토리를 생성합니다.
/// </summary>
/// <returns></returns>
void Graphics::GraphicsDevice::CreateDXGIFactory()
{
	// CreateDXGIFactory2()
	// : DXGI 팩토리를 생성하는 함수입니다.
	// 이 함수는 DXGIDebug.dll 로드되는지 여부를 나타내는 플래그를 허용합니다.
	// 그렇지 않으면 함수가 CreateDXGIFactory1과 동일하게 동작합니다.
	ThrowIfFailed(CreateDXGIFactory2(
		m_dxgiFactoryFlags,				// DXGI 팩토리 플래그
		IID_PPV_ARGS(&m_dxgiFactory)	// DXGI 팩토리 인터페이스를 요청합니다.
	));
}


/// <summary>
/// 하드웨어	어뎁터를 얻어옵니다.
/// 
/// </summary>
bool Graphics::GraphicsDevice::GetHardwareAdapter()
{
	// 성능순으로gpu어뎁터를 얻어오기 위해서는
	// IDXGIFactory6 인터페이스의 EnumAdapterByGpuPreference() 메서드를 사용해야 합니다.
	for (UINT adapterIndex = 0; ; ++adapterIndex)
	{
		// EnumAdapterByGpuPreference()
		// : 이 메서드는 지정된 GPU 선호도에 따라 어댑터를 열거합니다.
		if (DXGI_ERROR_NOT_FOUND ==
			m_dxgiFactory->EnumAdapterByGpuPreference(
				adapterIndex,
				DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
				IID_PPV_ARGS(&m_adpter)))
		{
			return false; // 더 이상 어댑터가 없으면 종료
		}

		// DXGI_ADAPTER_DESC1
		// DXGI 1.1을 사용하는 어댑터(또는 비디오 카드)에 대해 설명합니다.
		DXGI_ADAPTER_DESC1 desc;

		// GetDesc1()
		// : 어댑터의 설명을 가져옵니다.
		// 형식: DXGI_ADAPTER_DESC1*
		// 어댑터를 설명하는 DXGI_ADAPTER_DESC1 구조체에 대한 포인터입니다.
		// 이 매개 변수는 NULL이 아니어야 합니다.
		// "소프트웨어 어댑터"에 대해 0을 반환합니다.
		ThrowIfFailed(m_adpter->GetDesc1(&desc));

		// 소프트웨어 플래그가 설정되어 있으면 
		// 무시합니다.
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		return true; // 하드웨어 어댑터를 찾았으면 true 반환
	}
}


/// <summary>
/// 랜더링 디바이스를 생성합니다.
/// 
/// </summary>
/// <returns>하드웨어 어뎁터 사용 여부입니다.</returns>
void Graphics::GraphicsDevice::CreateDevice(const bool& hardwareAdapter)
{
	const D3D_FEATURE_LEVEL FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
		D3D_FEATURE_LEVEL_1_0_CORE,
		D3D_FEATURE_LEVEL_1_0_GENERIC,
	};

	if (!hardwareAdapter)
	{
		ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&m_adpter)));
	}

	// D3D12CreateDevice()
	// : Direct3D 12 디바이스를 생성합니다.
	// D3D_FEATURE_LEVEL enum을 순회하여
	// 지원되는 기능 수준을 지정합니다.
	for (const auto& option : FeatureLevels)
	{
		HRESULT hr = D3D12CreateDevice(m_adpter.Get(),	// 하드웨어 어댑터
			option,						// 요구되는 기능 수준
			IID_PPV_ARGS(&m_device));	// 디바이스 인터페이스를 요청합니다.

		if (SUCCEEDED(hr)) break; // 성공하면 루프 종료
	}

}

/// <summary>
/// 커맨드 큐를 생성합니다.
/// </summary>
/// <returns></returns>
void Graphics::GraphicsDevice::CreateCommandQueue()
{
	// D3D12_COMMAND_QUEUE_DESC
	// D3D12_COMMAND_LIST_TYPE Type : 어떤 명령을 처리할 큐인지 지정
	// INT Priority : 큐의 우선순위를 지정합니다.
	// D3D12_COMMAND_QUEUE_FLAGS Flags : 큐의 플래그를 지정합니다.
	// UINT NodeMask : 멀티 GPU 시스템에서 큐가 실행될 노드를 지정합니다.(멀티로 안쓰면 0)
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
}


/// <summary>
/// 동기화 객체 Fence를 생성합니다.
/// 
/// 이 객체는 GPU와 CPU 간의 동기화를 위해 사용됩니다.
/// </summary>
/// <returns></returns>
void Graphics::GraphicsDevice::CreateFence()
{
	// CreateFence() 메서드는 GPU와 CPU 간의 동기화를 위해 사용되는 Fence 객체를 생성합니다.
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValue = 0; // 초기 펜스 값 설정	

	// CreateEvent() 메서드는 CPU가 GPU의 작업 완료를 기다릴 때 사용할 이벤트 객체를 생성합니다.
	/*윈도우 표준 이벤트 오브젝트 생성

		CPU가 “이벤트가 발생할 때까지 기다리게” 할 수 있음

		Fence와 연동해서,
		→ GPU가 특정 작업 끝날 때까지 CPU를 잠시 멈추는 데 사용*/
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	if (m_fenceEvent == nullptr)
	{
		HRESULT_FROM_WIN32(GetLastError());
	}
}

/// <summary>
/// 지정된 펜스 값으로 명령 rmr큐에 신호를 보냅니다.
/// 이 메서드는 GPU가 특정 작업을 완료했음을 CPU에 알리는 데 사용됩니다.
/// 
/// </summary>
/// <param name="fenceValue">신호를 보낼 펜스 값입니다.</param>
void Graphics::GraphicsDevice::SignalFence(const UINT64& fenceValue)
{
	m_commandQueue->Signal(m_fence.Get(), fenceValue);
}

/// <summary>
/// 지정된 펜스 값에 도달할 때까지 렌더러를 대기시킵니다.
/// 이 메서드는 GPU가 특정 작업을 완료할 때까지 CPU를 대기시키는 데 사용됩니다.
/// 
/// </summary>
/// <param name="fenceValue">대기할 목표 펜스 값입니다.</param>
void Graphics::GraphicsDevice::WaitForFence(const UINT64& fenceValue)
{
 	if (m_fence.Get()->GetCompletedValue() < fenceValue)
	{
		m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}
