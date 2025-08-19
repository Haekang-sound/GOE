#include "Engine_pch.h"
#include "Scene.h"
#include "Entity.h"

#include "EntityManager.h"

#include "Transform.h"
#include "MeshRenderer.h"

#include "RenderSystem.h"

Scene::Scene() = default;
Scene::~Scene() = default;

void Scene::Initialize(GOE::EngineContext* context)
{
	m_context = context;

	m_entityManager = std::make_unique<EntityManager>();
	m_transformManager = std::make_unique<ComponentManager<Transform>>();
	m_meshRendererManager = std::make_unique<ComponentManager<MeshRenderer>>();

	m_renderSystem = std::make_unique<RenderSystem>(this, context);

	/// 씬데이터를 로딩하는거라고 생각하자
	Script();
	m_renderSystem.get()->Initialize();
	///이쯤 콘스탄트 버퍼를 생성하는것이 좋을것 같다.
	/// 그렇지 않으면 업데이트에서 콘스탄트 버퍼를 생성하게 된다.
	

}

void Scene::OnUpdate(double dTime)
{
	///1. 랜더시스템
	// 트랜스폼의 변환행렬을 통해 cb를 업데이트
	// 메쉬랜더러의 데이터정보를 확인해서 그걸 그리게 해야지
	m_renderSystem.get()->Update();

	///2. movement시스템
	// 인풋을 통해 트랜스폼을 업데이트한다.

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

		// 엔티티에 컴포넌트를 추가한다.
		// 컴포넌트를 추가할때 어떤일이 일어날지 생각해보자

		// 트랜스폼은 
		// 시작위치, 스케일, 회전을 넣을 수 있어야 함
		m_transformManager.get()->AddComponent(
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
			0);

		GOE::Matrix4x4 tm = {};
		tm._11 = 1;
		tm._22 = 1;
		tm._33 = 1;
		tm._44 = 1;

		m_transformManager.get()->GetCurrentComponent().SetLocalTM(tm);

		std::hash<std::string> hasher;
		// 메쉬랜더러는 메쉬id를 전달하는 구간이 필요하다.
		m_meshRendererManager.get()->AddComponent(m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(), 0);
		size_t meshPath = hasher("Ch03");

		std::string name = m_context->assetCore->GetMesh(meshPath)->GetName();
		size_t meshid = m_context->assetCore->GetMesh(meshPath)->GetID();
		size_t meshidx = m_context->assetCore->GetMesh(meshPath)->GetMeshIndex();
		size_t modelid = m_context->assetCore->GetMesh(meshPath)->GetModelID();

		m_meshRendererManager.get()->GetCurrentComponent().SetMeshID(m_context->assetCore->GetMesh(meshPath)->GetID());
		m_meshRendererManager.get()->GetCurrentComponent().SetMeshIndex(m_context->assetCore->GetMesh(meshPath)->GetMeshIndex());
		m_meshRendererManager.get()->GetCurrentComponent().SetModelID(m_context->assetCore->GetMesh(meshPath)->GetModelID());
		m_meshRendererManager.get()->GetCurrentComponent().SetName(m_context->assetCore->GetMesh(meshPath)->GetName());

	}

	{
		// 엔티티를 만들고
		m_entityManager.get()->CreateEntity("모델");

		// 트랜스폼은 
		// 시작위치, 스케일, 회전을 넣을 수 있어야 함
		m_transformManager.get()->AddComponent(
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
			0);

		GOE::Matrix4x4 tm = {};
		tm._11 = 1;
		tm._22 = 1;
		tm._33 = 1;
		tm._44 = 1;
		tm._41 = 100.0f; // 위치를 약간 이동시켜보자

		m_transformManager.get()->GetCurrentComponent().SetLocalTM(tm);

		std::hash<std::string> hasher;
		// 메쉬랜더러는 메쉬id를 전달하는 구간이 필요하다.
		m_meshRendererManager.get()->AddComponent(m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(), 0);
		size_t meshPath = hasher("Ch03");

		std::string name = m_context->assetCore->GetMesh(meshPath)->GetName();
		size_t meshid = m_context->assetCore->GetMesh(meshPath)->GetID();
		size_t meshidx = m_context->assetCore->GetMesh(meshPath)->GetMeshIndex();
		size_t modelid = m_context->assetCore->GetMesh(meshPath)->GetModelID();

		m_meshRendererManager.get()->GetCurrentComponent().SetMeshID(m_context->assetCore->GetMesh(meshPath)->GetID());
		m_meshRendererManager.get()->GetCurrentComponent().SetMeshIndex(m_context->assetCore->GetMesh(meshPath)->GetMeshIndex());
		m_meshRendererManager.get()->GetCurrentComponent().SetModelID(m_context->assetCore->GetMesh(meshPath)->GetModelID());
		m_meshRendererManager.get()->GetCurrentComponent().SetName(m_context->assetCore->GetMesh(meshPath)->GetName());
	}
}