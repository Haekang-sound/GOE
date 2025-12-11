#pragma once
#include "ID3DRenderer.h"
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
	struct RenderContext;

	class GraphicsDevice;
	class SwapChain;
	class PSOManager;
	class RenderCommandContext;
	class CopyCommandContext;
	class UIManager;
	class ResourceManager;
	class DescriptorHeapManager;
}

class Mesh;

using Microsoft::WRL::ComPtr;
using namespace Microsoft::WRL;

class Camera;
struct UIInitInfo;
struct UILoopInfo;
class ModelResource;
class MeshResource;
//class TextureResource;
class RenderObject;

/// <summary>
/// 인터페이스를 상속받아 구현된 랜더러
/// 
/// ohk 2025.06.21
/// </summary>
class GOERenderer : public GOE::ID3DRenderer
{
public:
	GOERenderer(const HWND hWnd);
	~GOERenderer();
	GOERenderer(const GOERenderer&) = delete;
	GOERenderer(GOERenderer&&) = delete;
	GOERenderer& operator=(const GOERenderer&) = delete;
	GOERenderer& operator=(GOERenderer&&) = delete;

protected:
	std::unique_ptr<Graphics::RenderContext> m_renderContext = nullptr;
	std::unique_ptr<Graphics::GraphicsDevice> m_graphicsDevice = nullptr;
	std::unique_ptr<Graphics::SwapChain> m_swapChain = nullptr;
	std::unique_ptr<Graphics::PSOManager> m_PSOManager = nullptr;
	std::unique_ptr<Graphics::ResourceManager> m_resourceManager = nullptr;
	std::unique_ptr<Graphics::UIManager> m_UIManager = nullptr;
	std::unique_ptr<Graphics::DescriptorHeapManager> m_descriptorHeapManager = nullptr;
	std::unique_ptr<Graphics::RenderCommandContext> m_commandContext = nullptr;
	std::unique_ptr<Graphics::CopyCommandContext> m_copyCommandContext = nullptr;

protected:
	std::vector<RenderObject> m_renderObjects;

	struct CameraData
	{
		float fov;
		float aspectRatio;
		float nearZ;
		float farZ;
		Graphics::Matrix4x4 worldMatrix;
		Graphics::FLoatVector3 position;
	} m_cameraData;

protected:
	HWND m_hWnd;

public:
	void OnInit() override;
	void OnUpdate(double dTime) override;
	void BeginRender() override;
	void OnRender() override;
	void EndRender() override;
	void OnDestroy() override;

public:
	void LoadTexture(std::string filepath) override;
	void CreateMeshResource(const Mesh* core_mesh) override;

public:
	UIInitInfo* GetUIInfo() override;
	UILoopInfo* GetUILoopInfo() override;
	void ReceiveRenderObejcts(std::vector<RenderObject>&& data) override;
	void SetCameraData(float fov, float aspect, float nZ, float fZ, const GOE::Matrix4x4& worldMat, const GOE::FLoatVector3& pos) override
	{
		m_cameraData.fov = fov;
		m_cameraData.aspectRatio = aspect;
		m_cameraData.nearZ = nZ;
		m_cameraData.farZ = fZ;
		m_cameraData.worldMatrix = worldMat;
		m_cameraData.position = pos;
	}
};
