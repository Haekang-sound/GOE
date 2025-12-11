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
		virtual void OnUpdate(double dTime) = 0;
		virtual void BeginRender() = 0;
		virtual void OnRender() = 0;
		virtual void EndRender() = 0;
		virtual void OnDestroy() = 0;

	public:
		// DirectX
		virtual void LoadTexture(std::string filepath) {};
		virtual void CreateMeshResource(const Mesh* core_mesh) {};
		virtual UIInitInfo* GetUIInfo() = 0;
		virtual UILoopInfo* GetUILoopInfo() = 0;
		virtual void SetCameraData(float fov, float aspect, float nZ, float fZ, const GOE::Matrix4x4& worldMat, const GOE::FLoatVector3& pos) = 0;

	public:
		virtual void ReceiveRenderObejcts(std::vector<RenderObject>&& data) = 0;
	};
}

