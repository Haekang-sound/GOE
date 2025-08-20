#pragma once
#include "ID3DRenderer.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

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
class Camera;
class UIInitInfo;
class UILoopInfo;
class ModelResource;
class MeshResource;
class RenderObject;

/// <summary>
/// 인터페이스를 상속받아 구현된 랜더러
/// 
/// ohk 2025.06.21
/// </summary>
class GOERenderer : public GOE::ID3DRenderer
{
public:
	GOERenderer(const HWND& hWnd);
	~GOERenderer();
	GOERenderer(const GOERenderer&) = delete;
	GOERenderer(GOERenderer&&) = delete;
	GOERenderer& operator=(const GOERenderer&) = delete;
	GOERenderer& operator=(GOERenderer&&) = delete;

public:
	void OnInit() override;
	void OnUpdate() override;
	void BeginRender() override;
	void OnRender() override;
	void EndRender() override;
	void OnDestroy() override;


public:
	void CreateAllMeshResources(const std::unordered_map<std::size_t, std::unique_ptr<Mesh>>& core_meshes) override;
	void CreateOneMeshResource(const Mesh* core_mesh) override;
	void CopyUploadHeapToDefault() override;
	UIInitInfo* GetUIInfo() override;
	UILoopInfo* GetUILoopInfo() override;
	void AddRenderObejct(RenderObjectData& data) override;

public:
	std::vector<std::unique_ptr<RenderObject>>& GetRenderObjects() override { return m_renderObjects; }

public:
	inline ID3D12Device* GetDevice() { return m_device.Get(); }
	inline ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }
	inline int GetFrameCount() { return m_frameBufferCount; }
	inline ID3D12DescriptorHeap* GetDescriptorHeap() { return m_imguiDescriptorHeap.Get(); }
	inline ID3D12GraphicsCommandList* GetCommandList() { return m_commandList.Get(); }
	inline ID3D12Resource* GetCurrentRendertarget() { return m_renderTargets[m_frameIndex].Get(); }

#pragma region Init
private:
	void SetViewport();
	void LoadPipeline();
	void LoadAssets();

public:
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
	

private:
	void SignalFence(const UINT64& fenceValue);
	void WaitForFence(const UINT64& fenceValue);

private:
	void CreateImguiDescriptorHeap();
#pragma endregion

public:
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
		RenderObject* render_object,
		const D3D12_RESOURCE_STATES& state = D3D12_RESOURCE_STATE_GENERIC_READ);

private:
	std::unordered_map<size_t, std::unique_ptr<MeshResource>> m_meshResources;
	/// <summary>
	/// 인스턴싱 단계에서 
	/// 랜더오브젝트 벡터는 깊은 고민이 필요할것이다.
	/// 
	/// </summary>
	std::vector<std::unique_ptr<RenderObject>> m_renderObjects;

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

	/// <summary>
	/// imgui를 위한 디스크립터힙
	/// </summary>
private:
	ComPtr<ID3D12DescriptorHeap> m_imguiDescriptorHeap = nullptr;
	std::unique_ptr<UILoopInfo> m_UILoopInfo;
	std::unique_ptr<UIInitInfo> m_UIInitInfo;
};
