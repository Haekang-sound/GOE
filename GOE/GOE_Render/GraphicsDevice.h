#pragma once
#include <wrl.h>

namespace Graphics
{
	/// <summary>
	/// D3D12의 핵심 객체(Device, Factory, Queue, Fence)를 소유하고 관리합니다.
	/// 엔진 전체에서 단 하나만 존재하는 싱글톤 또는 핵심 객체로 관리됩니다.
	/// 
	/// ohk
	/// </summary>
	class GraphicsDevice
	{
	public:
		GraphicsDevice() = default;
		~GraphicsDevice();
		GraphicsDevice(const GraphicsDevice&) = delete;
		GraphicsDevice(GraphicsDevice&&) = delete;
		GraphicsDevice& operator=(const GraphicsDevice&) = delete;
		GraphicsDevice& operator=(GraphicsDevice&&) = delete;

	public:
		void Initialize(const bool& useWarpDevice = false, const bool& debugLayerOn = false);

		void ActiveDebugLayer(const bool& isOn);
		void CreateDXGIFactory();
		bool GetHardwareAdapter();
		void CreateDevice(const bool& hardwareAdapter);
		void CreateCommandQueue();

		void CreateFence();

		void SignalFence();
		void WaitForFence();

	
		UINT m_dxgiFactoryFlags = 0;
		ComPtr<IDXGIFactory6> m_dxgiFactory = nullptr;
		ComPtr<ID3D12Device> m_device = nullptr;
		bool m_useWarpDevice = false;
		HANDLE m_fenceEvent = nullptr; 
		UINT64 m_fenceValue = 0;


		ComPtr<IDXGIAdapter1> m_adpter = nullptr;
		ComPtr<ID3D12CommandQueue> m_commandQueue = nullptr;
		ComPtr<ID3D12Fence> m_fence = nullptr;		
	};
}


