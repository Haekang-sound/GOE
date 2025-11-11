#pragma once

namespace Graphics
{
	class SwapChain
	{
	public:
		SwapChain(const HWND hWnd, const UINT frameBufferCount);
		SwapChain() = delete;
		~SwapChain();
		SwapChain(const SwapChain&) = delete;
	public:
		D3D12_VIEWPORT m_viewport = {};
		RECT m_scissorRect = {};
		float m_aspectRatio = 0.f;
		
		UINT m_width = 0;
		UINT m_height = 0;

		static const UINT m_frameBufferCount = 2;
		//UINT m_frameBufferCount = 2;

		UINT m_frameIndex = 0;
		ComPtr<IDXGISwapChain3> m_swapChain = nullptr;
		ComPtr<ID3D12Resource> m_renderTargets[m_frameBufferCount] = {};
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap = nullptr;
		UINT m_rtvDescriptorSize = 0;

		ComPtr<ID3D12Resource> m_depthStencilBuffer = nullptr;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap = nullptr;

		// 프로젝션 행렬
		XMFLOAT4X4 m_proj = {};

	public:
		void Initialize();

	private:
		void CreateSwapChain();
		void CreateRTVHeap();
		void CreateRenderTargets();
		void CreateDepthStencilBuffer();
		void SetViewport();

	private:
		HWND m_hWnd;

	};
}

