#include "Engine_pch.h"
#include "Scene.h"
#include "Entity.h"

#include "EntityManager.h"

#include "Transform.h"
#include "MeshRenderer.h"
#include "MovementUnit.h"

#include "RenderSystem.h"
#include "MovementSystem.h"
#include "TransfromSystem.h"

Scene::Scene() = default;
Scene::~Scene() = default;

void Scene::Initialize(GOE::EngineContext* context)
{
	m_context = context;

	// 매니저 생성
	m_entityManager = std::make_unique<EntityManager>();
	m_transformManager = std::make_unique<ComponentManager<Transform>>();
	m_meshRendererManager = std::make_unique<ComponentManager<MeshRenderer>>();
	m_movementUnitManager = std::make_unique<ComponentManager<MovementUnit>>();
	// 시스템 생성
	m_renderSystem = std::make_unique<RenderSystem>(this, context);
	m_transfromSystem = std::make_unique<TransfromSystem>(this, context);
	m_movementSystem = std::make_unique<MovementSystem>(this, context);

	Script();

	// 랜더오브젝트를 만드는 랜더시스템 초기화
	m_renderSystem.get()->Initialize();
}

void Scene::OnUpdate(double dTime)
{
	m_movementSystem.get()->Update(dTime);
	m_transfromSystem.get()->Update(dTime);
	m_renderSystem.get()->Update(dTime);
}

void Scene::DebugUpdate()
{
	m_movementSystem.get()->DebugUpdate(0);
	m_transfromSystem.get()->DebugUpdate(0);
	m_renderSystem.get()->DebugUpdate(0);
}

/// <summary>
/// 현재는 함수로 구현됐지만 
/// 나중에는 데이터가 될 스크립트
/// </summary>
void Scene::Script()
{
	{
		// 엔티티를 만들고
		m_entityManager.get()->CreateEntity("모델");

		// 트랜스폼은 
		// 시작위치, 스케일, 회전을 넣을 수 있어야 함
		m_transformManager.get()->AddComponent(
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID());

		GOE::Matrix4x4 tm = GOE::Matrix4x4::Identity();
		m_transformManager.get()->GetCurrentComponent().SetLocalTM(tm);
		m_transformManager.get()->GetCurrentComponent().SetScaleTM({ 100.f, 100.f, 100.f });

		std::hash<std::string> hasher;
		// 메쉬랜더러는 메쉬id를 전달하는 구간이 필요하다.
		// 일단 컴포넌트 아이디와 entity 아이디를 동일하게 사용한다.
		m_meshRendererManager.get()->AddComponent(
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID());
		size_t meshPath = hasher("chr629_0");

		m_meshRendererManager.get()->GetCurrentComponent().SetMeshID(m_context->assetCore->GetMesh(meshPath)->GetID());
		m_meshRendererManager.get()->GetCurrentComponent().SetMeshIndex(m_context->assetCore->GetMesh(meshPath)->GetMeshIndex());
		m_meshRendererManager.get()->GetCurrentComponent().SetModelID(m_context->assetCore->GetMesh(meshPath)->GetModelID());
		m_meshRendererManager.get()->GetCurrentComponent().SetName(m_context->assetCore->GetMesh(meshPath)->GetName());

		m_movementUnitManager.get()->AddComponent(
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID());
	}

	{
		// 엔티티를 만들고
		m_entityManager.get()->CreateEntity("모델2");

		// 트랜스폼은 
		// 시작위치, 스케일, 회전을 넣을 수 있어야 함
		m_transformManager.get()->AddComponent(
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID());

		GOE::Matrix4x4 tm = GOE::Matrix4x4::Identity();
		tm._41 = 40.0f; // 위치를 약간 이동시켜보자

		m_transformManager.get()->GetCurrentComponent().SetLocalTM(tm);

		std::hash<std::string> hasher;
		// 메쉬랜더러는 메쉬id를 전달하는 구간이 필요하다.
		m_meshRendererManager.get()->AddComponent(m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID());
		size_t meshPath = hasher("Ch03");

		m_meshRendererManager.get()->GetCurrentComponent().SetMeshID(m_context->assetCore->GetMesh(meshPath)->GetID());
		m_meshRendererManager.get()->GetCurrentComponent().SetMeshIndex(m_context->assetCore->GetMesh(meshPath)->GetMeshIndex());
		m_meshRendererManager.get()->GetCurrentComponent().SetModelID(m_context->assetCore->GetMesh(meshPath)->GetModelID());
		m_meshRendererManager.get()->GetCurrentComponent().SetName(m_context->assetCore->GetMesh(meshPath)->GetName());
	}
}