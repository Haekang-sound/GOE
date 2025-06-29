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

#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;
using namespace Microsoft::WRL;
using namespace DirectX;

struct MVP
{
	DirectX::XMFLOAT4X4 mvp; // 64바이트(행렬), row-major/col-major는 HLSL에서 맞춰줌
};


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

	void CreateIndexBuffer();
	void CreateConstantBuffer();
	ComPtr<ID3D12Resource> m_constantBuffer;

	void CreateFence();
	void CopyUploadHeapToDefault();

private:
	void PopulateCommandList();
	void SignalFence(const UINT64& fenceValue);
	void WaitForFence(const UINT64& fenceValue);

private:
	float m_angle = 0.0f;

	XMFLOAT3 RotateCubeVertex(float x, float y, float z)
	{
		// Y축 45도, X축 30도
		XMMATRIX rotY = XMMatrixRotationY(XMConvertToRadians(45.0f));
		XMMATRIX rotX = XMMatrixRotationX(XMConvertToRadians(30.0f));
		XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
		pos = XMVector3TransformCoord(pos, rotY * rotX);
		XMFLOAT3 out;
		XMStoreFloat3(&out, pos);
		return out;
	}

	inline void ThrowIfFailed(const HRESULT& hr)
	{
		if (FAILED(hr))
		{
			throw std::runtime_error("HRESULT failed!");
		}
	}

private:
	UINT m_width = 0;
	UINT m_height = 0;
	HWND m_hWnd;

	bool m_useWarpDevice = false;

private:
	ComPtr<ID3D12Resource> m_indexBufferUpload = nullptr;
	ComPtr<ID3D12Resource> m_indexBufferDefault = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};
	UINT m_indexBufferSize = 0;


private:
	static const UINT m_frameBufferCount = 2;

	D3D12_VIEWPORT m_viewport = {};
	RECT m_scissorRect = {};
	float m_aspectRatio;
	// 올바른 코드 예시
	float aspect = (float)m_width / (float)m_height;
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), aspect, 0.1f, 100.0f);


	UINT m_dxgiFactoryFlags = 0;
	ComPtr<IDXGIFactory6> m_dxgiFactory = nullptr;
	ComPtr<IDXGISwapChain3> m_swapChain = nullptr;;
	ComPtr<IDXGIAdapter1> m_adpter = nullptr;;
	ComPtr<ID3D12Device> m_device = nullptr;;

	ComPtr<ID3D12Resource> m_renderTargets[m_frameBufferCount] = {};
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap = nullptr;;
	UINT m_rtvDescriptorSize = 0;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;;
	ComPtr<ID3D12CommandQueue> m_commandQueue = nullptr;;
	
	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;;

	struct Vertex
	{
		XMFLOAT3 position;
		float color[4];
	};

	ComPtr<ID3DBlob> m_vertexShader = nullptr;;
	ComPtr<ID3DBlob> m_pixelShader = nullptr;;
	D3D12_INPUT_ELEMENT_DESC m_inputElementDescs[2] = {};
	Vertex m_triangleVertices[24] = {};
	UINT m_vertexBufferSize = 0;

	ComPtr<ID3D12Resource> m_vertexBufferUpload = nullptr;
	ComPtr<ID3D12Resource> m_vertexBufferDefault = nullptr;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

	ComPtr<ID3D12PipelineState> m_pipelineState = nullptr;
	ComPtr<ID3D12GraphicsCommandList> m_commandList = nullptr;

	UINT m_frameIndex = 0;
	HANDLE m_fenceEvent = nullptr;
	ComPtr<ID3D12Fence> m_fence = nullptr;

	// 프레임마다 동기화를 위한 fece값을 갖는다.
	UINT64 m_fenceValue = 0;
};
