#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <d3dx12/d3dx12_core.h>
#include <d3dx12/d3dx12_root_signature.h>
#include <d3dx12/d3dx12_barriers.h>

#include <windows.h>
#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <stdexcept>
#include <comdef.h>

#include "ID3DRenderer.h"

using Microsoft::WRL::ComPtr;
using namespace Microsoft::WRL;
using namespace DirectX;

/// <summary>
/// 인터페이스를 상속받아 구현된 랜더러
/// 
/// ohk 2025.06.21
/// </summary>
class GOERenderer : public ID3DRenderer
{
public:
	GOERenderer(HWND hWnd);

	void OnInit() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnDestroy() override;

public:
	void LoadPipeline();
	void LoadAssets();
	void PopulateCommandList();
	void WaitForPreviousFrame();
	void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter);

public: 
	/// 초기화를 명시적으로 이해하기 위해 
	/// 함수들을 좀더 자세히 분리했습니다.
	
	// 초기화 및 객체 생성
	HRESULT CreateDXGIFactory();
	HRESULT ActiveDebugLayer(bool isOn);
	HRESULT ChooseAdapter();
	HRESULT CreateDevice();
	HRESULT CreateCommandQueue();
	HRESULT CreateSwapChain();
	HRESULT CreateDescriptorHeaps();
	HRESULT CreateRenderTargets();
	HRESULT CreateCommandAllocator();
	HRESULT CreateRootSignature();
	HRESULT CompileShaders();
	HRESULT CreatePipelineState();
	HRESULT CreateCommandList();
	HRESULT CreateVertexBuffer();
	HRESULT SetVertexBufferView();
	HRESULT CreateFence();
	
public:
	// Viewport dimensions.
	UINT m_width;
	UINT m_height;
	HWND m_hWnd;

	float m_aspectRatio;
	bool m_useWarpDevice;

private:
	static const UINT FrameCount = 2;

	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};
	ComPtr<ID3DBlob> m_vertexShader;
	ComPtr<ID3DBlob> m_pixelShader;
	D3D12_INPUT_ELEMENT_DESC m_inputElementDescs[2];
	Vertex m_triangleVertices[3];
	UINT m_vertexBufferSize;
	

	// Pipeline objects.
	CD3DX12_VIEWPORT m_viewport;	// 뷰	
	CD3DX12_RECT m_scissorRect;		// 스크린 영역
	UINT m_dxgiFactoryFlags = 0;

	ComPtr<IDXGIFactory4> m_dxgiFactory;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12Device> m_device;

	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	UINT m_rtvDescriptorSize;

	// App resources.
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

	
};

