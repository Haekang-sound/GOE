#pragma once
#include <unordered_map>
#include <list>
#include <memory>

#include "IComponentManager.h"

namespace GOE
{
	struct EngineContext;
}

class Entity;
class EntityManager;

class Transform;
class MeshRenderer;

class RenderSystem;



/// <summary>
/// 씬의 내용이 기록되는 Scene 
/// 
/// </summary>
class Scene
{
public:
	Scene();
	~Scene();

	/// <summary>
	///  현재 씬에 존재하는 라이프사이클 함수는 
	///  나중에 Scenemanager선에서 관리되어야 한다.
	/// </summary>
public:
	void Initialize(GOE::EngineContext* context);

	void OnUpdate(double dTime);

	void BeginRender();
	void OnRender();
	void EndRender();

	void Release();

	void InputUpdate();
	void DebugUpdate();

public:
	inline const EntityManager* GetEntityManager() const { return m_entityManager.get();}
	inline const ComponentManager<Transform>* GetTransformManager() const { return m_transformManager.get(); }
	inline const ComponentManager<MeshRenderer>* GetMeshRendererManager()const { return m_meshRendererManager.get(); }


private:
	void Script();// 씬을 적는 곳이 있어야한다.
	
private:
	GOE::EngineContext* m_context = nullptr;

private: 
	// 컴포넌트들을 일괄처리할 system들이 필요함
	std::unique_ptr<EntityManager> m_entityManager;
	std::unique_ptr<ComponentManager<Transform>> m_transformManager;
	std::unique_ptr<ComponentManager<MeshRenderer>> m_meshRendererManager;

	std::unique_ptr<RenderSystem> m_renderSystem;
};

