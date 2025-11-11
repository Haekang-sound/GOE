#pragma once
#include "ID3DRenderer.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

#include <wrl.h>
#include <windows.h>
#include <memory>
#include <unordered_map>

namespace GOE
{
	struct MeshData;
}
namespace Graphics
{
	struct MeshData;
	class SwapChain;
}

class Mesh;

using Microsoft::WRL::ComPtr;
using namespace Microsoft::WRL;

class Camera;
struct UIInitInfo;
struct UILoopInfo;
class ModelResource;
class MeshResource;
class TextureResource;
class RenderObject;

/// <summary>
/// 인터페이스를 상속받아 구현된 랜더러
/// 
/// ohk 2025.06.21
/// </summary>
class GOERenderer : public GOE::ID3DRenderer
{
protected: 
	std::unique_ptr<Graphics::SwapChain> m_swapChain = nullptr;

public:
	GOERenderer(const HWND hWnd);
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
	void LoadTexture(std::string filepath) override;
	void CreateAllMeshResources(const std::unordered_map<std::size_t, std::unique_ptr<Mesh>>& core_meshes) override;
	void CreateOneMeshResource(const Mesh* core_mesh) override;
	void CopyUploadHeapToDefault() override;
	UIInitInfo* GetUIInfo() override;
	UILoopInfo* GetUILoopInfo() override;
	void AddRenderObejct(RenderObjectData& data) override;

public:
	std::vector<std::unique_ptr<RenderObject>>& GetRenderObjects() override { return m_renderObjects; }

protected:
	void LoadAssets();

public:
	void CreateCommandAllocator();
	void CreateRootSignature();
	void CompileShaders();
	void CreatePipelineState();
	void CreateCommandList();

protected:
	void CreateImguiDescriptorHeap();

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
		MeshResource* mesh_resource,
		const Graphics::MeshData& mesh_data,
		const D3D12_RESOURCE_STATES& state = D3D12_RESOURCE_STATE_GENERIC_READ);
	void CreateRenderObjectCBResource(
		RenderObject* render_object,
		const D3D12_RESOURCE_STATES& state = D3D12_RESOURCE_STATE_GENERIC_READ);
	HRESULT CompileShaderFromFile(const WCHAR* fileName, const WCHAR* entryPoint, const WCHAR* targetProfile, ID3DBlob** ppShaderBlob);



	/// <summary>
	/// renderer는 너무 많은 리소스를 직접 소유하고 있다.
	/// 그래픽스단위의 리소스매니저가 필요하다
	/// </summary>
private:
	std::vector<std::unique_ptr<MeshResource>> m_meshResources;
	std::unordered_map<size_t, MeshResource*> m_meshResourceMap;
	std::vector<std::unique_ptr<TextureResource>> m_textureResources;
	std::unordered_map<size_t, TextureResource*> m_textureResourceMap;
	std::vector<std::unique_ptr<RenderObject>> m_renderObjects;

public:
	Camera* m_camera;

private:
	HWND m_hWnd;

private:
	ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;
	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
	ComPtr<ID3DBlob> m_vertexShader = nullptr;
	ComPtr<ID3DBlob> m_pixelShader = nullptr;
	D3D12_INPUT_ELEMENT_DESC m_inputElementDescs[6] = {};
	ComPtr<ID3D12PipelineState> m_pipelineState = nullptr;
	ComPtr<ID3D12GraphicsCommandList> m_commandList = nullptr;

	/// <summary>
	/// imgui를 위한 디스크립터힙
	/// </summary>
private:
	ComPtr<ID3D12DescriptorHeap> m_imguiDescriptorHeap = nullptr;
	std::unique_ptr<UILoopInfo> m_UILoopInfo;
	std::unique_ptr<UIInitInfo> m_UIInitInfo;
};
