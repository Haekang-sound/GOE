#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

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
	~GOERenderer();

	void OnInit() override;
	void OnUpdate() override;
	void OnRender() override;
	void OnDestroy() override;

private:
	UINT m_width;
	UINT m_height;
	HWND m_hWnd;

	bool m_useWarpDevice;

private:
	static const UINT FrameCount = 2;	

	D3D12_VIEWPORT m_viewport;
	RECT m_scissorRect;
	float m_aspectRatio;

	UINT m_dxgiFactoryFlags = 0;
	ComPtr<IDXGIFactory6> m_dxgiFactory;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<IDXGIAdapter1> m_adpter;
	ComPtr<ID3D12Device> m_device;

	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	UINT m_rtvDescriptorSize;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<ID3D12RootSignature> m_rootSignature;

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

	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

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
	void CreateVertexBuffer();
	void SetVertexBufferView();
	void CreateFence();

private:
	void PopulateCommandList();
	void WaitForPreviousFrame();

};

