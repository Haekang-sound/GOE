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
class Material;
class MovementUnit;

class RenderSystem;
class TransfromSystem;
class MovementSystem;

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

	void DebugUpdate();
	void InputUpdate();

public:
	inline const EntityManager* GetEntityManager() const { return m_entityManager.get(); }
	inline const ComponentManager<Transform>* GetTransformManager() const { return m_transformManager.get(); }
	inline const ComponentManager<MeshRenderer>* GetMeshRendererManager()const { return m_meshRendererManager.get(); }
	inline const ComponentManager<MovementUnit>* GetMovementUnitManager() const { return m_movementUnitManager.get(); }
	inline ComponentManager<Transform>* GetTransformManager() { return m_transformManager.get(); }
	inline ComponentManager<MeshRenderer>* GetMeshRendererManager() { return m_meshRendererManager.get(); }
	inline ComponentManager<Material>* GetMaterialManager() { return m_materialManager.get(); }
	inline ComponentManager<MovementUnit>* GetMovementUnitManager() { return m_movementUnitManager.get(); }

private:
	void Script();// 씬을 적는 곳이 있어야한다.

private:
	GOE::EngineContext* m_context = nullptr;

private:
	// 컴포넌트들을 일괄처리할 system들이 필요함
	std::unique_ptr<EntityManager> m_entityManager;
	std::unique_ptr<ComponentManager<Transform>> m_transformManager;
	std::unique_ptr<ComponentManager<MeshRenderer>> m_meshRendererManager;
	std::unique_ptr<ComponentManager<Material>> m_materialManager;
	/// 이동을 한번 만들어볼 예정 이것까지 끝나면 정말 엔진구조는 잠시스탑
	std::unique_ptr<ComponentManager<MovementUnit>> m_movementUnitManager;

	std::unique_ptr<RenderSystem> m_renderSystem;
	std::unique_ptr<TransfromSystem> m_transfromSystem;
	std::unique_ptr<MovementSystem> m_movementSystem;
};

