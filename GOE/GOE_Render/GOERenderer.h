#pragma once
#include "ID3DRenderer.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <d3dx12/d3dx12.h>
#include <wrl.h>
#include <windows.h>
#include <memory>
#include <unordered_map>


namespace GOE
{
	struct MeshData;
}

using Microsoft::WRL::ComPtr;
using namespace Microsoft::WRL;

class Cube;
class Kuramon;
class Camera;
class UIInitInfo;
class UILoopInfo;

class ModelResource;
class MeshResource;

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

	// 복사생성자
	GOERenderer(const GOERenderer&) = delete;
	// 복사 대입 연산자
	GOERenderer& operator=(const GOERenderer&) = delete;
	// 이동생성자
	GOERenderer(GOERenderer&&) = delete;
	// 이동 대입 연산자
	GOERenderer& operator=(GOERenderer&&) = delete;


public:
	void OnInit() override;
	void OnUpdate() override;

	void BeginRender() override;
	void OnRender() override;
	void EndRender() override;

	void OnDestroy() override;

public:
	inline ID3D12Device* GetDevice() { return m_device.Get(); }
	inline ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }
	inline int GetFrameCount() { return m_frameBufferCount; }
	inline ID3D12DescriptorHeap* GetDescriptorHeap() { return m_imguiDescriptorHeap.Get(); }
	inline ID3D12GraphicsCommandList* GetCommandList() { return m_commandList.Get(); }
	inline ID3D12Resource* GetCurrentRendertarget() { return m_renderTargets[m_frameIndex].Get(); }

	UIInitInfo* GetUIInfo();
	UILoopInfo* GetUILoopInfo();

public:
	void CreateModelData(size_t id);

private:
	void SetViewport();

	void LoadPipeline();
	void LoadAssets();

public:
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

public:
	void CreateFence();
	void CopyUploadHeapToDefault();

private:
	void SignalFence(const UINT64& fenceValue);
	void WaitForFence(const UINT64& fenceValue);

private:
	void CreateImguiDescriptorHeap();

	/// <summary>
	/// core모델 to grpahics
	/// 모델 변환중
	/// </summary>
	/// <param name="core_models"></param>
public:
	void CreateAllModelResource(const std::unordered_map <std::size_t, std::unique_ptr<Model>>& core_models);
	void CreateMeshResource(MeshResource* mesh_resource, Graphics::MeshData& mesh_data);
	void CreateVBResource(
		MeshResource* mesh_resource,
		const Graphics::MeshData& mesh_data,
		const D3D12_RESOURCE_STATES& state = D3D12_RESOURCE_STATE_GENERIC_READ);
	void CreateIBResource(
		MeshResource* mesh_resource,
		const Graphics::MeshData& mesh_data,
		const D3D12_RESOURCE_STATES& state = D3D12_RESOURCE_STATE_GENERIC_READ);
	void CreateCBResource(
		MeshResource* mesh_resource,
		const Graphics::MeshData& mesh_data,
		const D3D12_RESOURCE_STATES& state = D3D12_RESOURCE_STATE_GENERIC_READ);

private:
	std::unordered_map<size_t, std::unique_ptr<ModelResource>> m_modelResources;

	/// <summary>
	///  얼른 오브젝트를 제대로 만들어서 없애야 하는부분
	/// </summary>
public:
	GOE::MeshData* GetKuramonMeshData();
	void TransVertexuramon();

	// 임시로 둔것
public:
	Camera* m_camera;
	Cube* m_cube;
	Kuramon* m_kuramon;

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
	std::unique_ptr<UILoopInfo> m_UILoopInfo;
	std::unique_ptr<UIInitInfo> m_UIInitInfo;
};
