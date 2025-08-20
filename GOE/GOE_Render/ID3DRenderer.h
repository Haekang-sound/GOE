#pragma once

class Model;
class UIInitInfo;
class UILoopInfo;
class RenderObjectData;
class RenderObject;

/// <summary>
/// 사용자가 다루게될 최전방의 인터페이스 
/// 
/// ohk 2025.06.21
/// </summary>
namespace GOE
{
	class ID3DRenderer
	{
	public:
		virtual ~ID3DRenderer() = default;

		virtual void OnInit() = 0;
		virtual void OnUpdate() = 0;
		virtual void BeginRender() = 0;
		virtual void OnRender() = 0;
		virtual void EndRender() = 0;
		virtual void OnDestroy() = 0;

		// DirectX
		virtual void CreateAllMeshResources(const std::unordered_map<std::size_t, std::unique_ptr<Mesh>>& core_meshes) {}
		virtual void CreateOneMeshResource(const Mesh* core_mesh) {};
		virtual void CopyUploadHeapToDefault() = 0;
		virtual UIInitInfo* GetUIInfo() = 0;
		virtual UILoopInfo* GetUILoopInfo() = 0;
	public:
		virtual std::vector<std::unique_ptr<RenderObject>>& GetRenderObjects() = 0;

	public:
		virtual void  AddRenderObejct(RenderObjectData& data) {};
	};
}

