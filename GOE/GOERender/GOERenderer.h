#pragma once
#include "ID3DRenderer.h"
#include "GOETypes.h"


using Microsoft::WRL::ComPtr;
using namespace Microsoft::WRL;
using namespace DirectX;

class Cube;
class Camera;

/// <summary>
/// 인터페이스를 상속받아 구현된 랜더러
/// 
/// ohk 2025.06.21
/// </summary>
class GOERenderer : public ID3DRenderer
{
public:
	GOERenderer(const HWND& hWnd);
	~GOERenderer();

	void OnInit() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnDestroy() override;

public: 
	inline ID3D12Device* GetDevice() { return m_device.Get(); }
	inline ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }
	inline int GetFrameCount() { return m_frameBufferCount; }
	inline ID3D12DescriptorHeap* GetDescriptorHeap() { return m_imguiDescriptorHeap.Get(); }
	inline ID3D12GraphicsCommandList* GetCommandList() { return m_commandList.Get(); }
	
private:
	void SetViewport();

	void LoadPipeline();
	void LoadAssets();

private:
	/// 초기화를 명시적으로 이해하기 위해 
	/// 함수들을 분리했습니다.
	void ActiveDebugLayer(const bool& isOn);
	void CreateDXGIFactory();
	bool GetHardwareAdapter();
	void CreateDevice(const bool& hardwareAdapter);
	void CreateCommandQueue();
	void CreateSwapChain();
	void CreateDescriptorHeaps();
	void CreateRenderTargets();
	void CreateCommandAllocator();
	void CreateRootSignature();
	void CompileShaders();
	void CreatePipelineState();
	void CreateCommandList();

	void CreateFence();
	void CopyUploadHeapToDefault();

private:
	void PopulateCommandList();
	void SignalFence(const UINT64& fenceValue);
	void WaitForFence(const UINT64& fenceValue);

private: 
	void CreateImguiDescriptorHeap();

	// 임시로 둔것에 가깝다
public: 
	Camera* m_camera;
	Cube* m_cube;

private:
	UINT m_width = 0;
	UINT m_height = 0;
	HWND m_hWnd;

private:
	D3D12_VIEWPORT m_viewport = {};
	RECT m_scissorRect = {};
	float m_aspectRatio;

	// 프로젝션 행렬
	XMFLOAT4X4 m_proj = {};

private:
	UINT m_dxgiFactoryFlags = 0;
	ComPtr<IDXGIFactory6> m_dxgiFactory = nullptr;
	bool m_useWarpDevice = false;
	ComPtr<IDXGIAdapter1> m_adpter = nullptr;
	ComPtr<ID3D12Device> m_device = nullptr;

	static const UINT m_frameBufferCount = 2;
	UINT m_frameIndex = 0;
	ComPtr<IDXGISwapChain3> m_swapChain = nullptr;

	ComPtr<ID3D12Resource> m_renderTargets[m_frameBufferCount] = {};
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap = nullptr;
	UINT m_rtvDescriptorSize = 0;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;
	ComPtr<ID3D12CommandQueue> m_commandQueue = nullptr;

	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
	ComPtr<ID3DBlob> m_vertexShader = nullptr;
	ComPtr<ID3DBlob> m_pixelShader = nullptr;

	D3D12_INPUT_ELEMENT_DESC m_inputElementDescs[2] = {};

	ComPtr<ID3D12PipelineState> m_pipelineState = nullptr;
	ComPtr<ID3D12GraphicsCommandList> m_commandList = nullptr;

	HANDLE m_fenceEvent = nullptr;
	ComPtr<ID3D12Fence> m_fence = nullptr;

	// 프레임마다 동기화를 위한 fece값을 갖는다.
	UINT64 m_fenceValue = 0;

private: 
	ComPtr<ID3D12DescriptorHeap> m_imguiDescriptorHeap = nullptr;
};
