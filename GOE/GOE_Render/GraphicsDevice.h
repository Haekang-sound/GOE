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

	public:
		void SignalRenderFence();
		void SignalCopyFence();
		void WaitForRenderFence();
		void WaitForCopyFence();
		void WaitForRenderFenceValue(const UINT64& fenceValue);
		void WaitForCopyFenceValue(const UINT64& fenceValue);

		void WaitForRenderQueue();
		void WaitForCopyQueue();
		void WaitForRenderQueueValue(const UINT64 fenceValue);
		void WaitForCopyQueueValue(const UINT64 fenceValue);

	public:
		HANDLE GetRenderFenceEvent() const { return m_renderFenceEvent; }
		HANDLE GetCopyFenceEvent() const { return m_copyFenceEvent; }
		UINT64 GetRenderFenceValue() const { return m_renderFenceValue; }
		UINT64 GetCopyFenceValue() const { return m_copyFenceValue; }
		// 현재 까지 완료된 펜스 값 반환
		UINT64 GetCompletedRenderFenceValue() const { return m_renderFence->GetCompletedValue(); }
		UINT64 GetCompletedCopyFenceValue() const { return m_copyFence->GetCompletedValue(); }

	protected:
		void ActiveDebugLayer(const bool& isOn);
		void CreateDXGIFactory();
		bool GetHardwareAdapter();
		void CreateDevice(const bool& hardwareAdapter);
		void CreateCommandQueue();
		void CreateFence();

	public:
		UINT m_dxgiFactoryFlags = 0;
		ComPtr<IDXGIFactory6> m_dxgiFactory = nullptr;
		ComPtr<ID3D12Device> m_device = nullptr;
		bool m_useWarpDevice = false;
		ComPtr<IDXGIAdapter1> m_adpter = nullptr;

		ComPtr<ID3D12CommandQueue> m_renderCmdQueue = nullptr;
		ComPtr<ID3D12CommandQueue> m_copyCmdQueue = nullptr;

	private:
		// Rendering 
		ComPtr<ID3D12Fence> m_renderFence = nullptr;
		UINT64 m_renderFenceValue = 0;
		HANDLE m_renderFenceEvent = nullptr;

		// Copy 
		ComPtr<ID3D12Fence> m_copyFence = nullptr;
		UINT64 m_copyFenceValue = 0;
		HANDLE m_copyFenceEvent = nullptr;

	};
}


