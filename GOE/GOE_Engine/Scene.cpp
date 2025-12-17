#include "Engine_pch.h"
#include "Scene.h"
#include "Entity.h"

#include "EntityManager.h"

#include "Transform.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "MovementUnit.h"
#include "AnimationUnit.h"
#include "CameraComponent.h"

#include "RenderSystem.h"
#include "MovementSystem.h"
#include "TransfromSystem.h"
#include "AnimationSystem.h"

Scene::Scene() = default;
Scene::~Scene() = default;

void Scene::Initialize(GOE::EngineContext* context)
{
	m_context = context;

	// 매니저 생성
	m_entityManager = std::make_unique<EntityManager>();
	m_transformManager = std::make_unique<ComponentManager<Transform>>();
	m_meshRendererManager = std::make_unique<ComponentManager<MeshRenderer>>();
	m_materialManager = std::make_unique<ComponentManager<Material>>();
	m_movementUnitManager = std::make_unique<ComponentManager<MovementUnit>>();
	m_animationUnitManager = std::make_unique<ComponentManager<AnimationUnit>>();
	m_cameraManager = std::make_unique<ComponentManager<CameraComponent>>();

	// 시스템 생성
	m_renderSystem = std::make_unique<RenderSystem>(this, context);
	m_transfromSystem = std::make_unique<TransfromSystem>(this, context);
	m_movementSystem = std::make_unique<MovementSystem>(this, context);
	m_animationSystem = std::make_unique<AnimationSystem>(this, context);

	Script();

	// 랜더오브젝트를 만드는 랜더시스템 초기화
	m_renderSystem.get()->Initialize();
}

void Scene::OnUpdate(double dTime)
{
	m_movementSystem.get()->Update(dTime);
	m_transfromSystem.get()->Update(dTime);
	m_animationSystem.get()->Update(dTime);
	m_renderSystem.get()->Update(dTime);
}

void Scene::DebugUpdate(double dTime)
{
	m_movementSystem.get()->DebugUpdate(dTime);
	m_transfromSystem.get()->DebugUpdate(dTime);
	m_animationSystem.get()->DebugUpdate(dTime);
	m_renderSystem.get()->DebugUpdate(dTime);
}

/// <summary>
/// 현재는 함수로 구현됐지만 
/// 나중에는 데이터가 될 스크립트
/// </summary>
void Scene::Script()
{
	{
		// 카메라 엔티티 생성
		m_entityManager.get()->CreateEntity("메인카메라");
		size_t cameraID = m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID();

		// Transform 추가
		m_transformManager.get()->AddComponent(cameraID, cameraID);
		auto transform = m_transformManager.get()->GetCurrentComponent();
		transform->SetLocalTM(GOE::Matrix4x4::Identity());
		// 초기 위치 설정 (기존 Camera.cpp의 m_position = {0.f, 15.f, -36.f})
		transform->SetPositionTM({ 0.0f, 15.0f, -36.0f });

		// CameraComponent 추가
		m_cameraManager.get()->AddComponent(cameraID, cameraID);

		// MovementUnit 추가 (카메라 이동을 위해)
		m_movementUnitManager.get()->AddComponent(cameraID, cameraID);
		m_movementUnitManager.get()->GetCurrentComponent()->SetMoveable(true);
	}

	{
		/// 엔티티를 만들고 부모자식관계를 하이어라키에서 확인하기위한 테스느
		m_entityManager.get()->CreateEntity("믹사모");
		m_entityManager.get()->SetParent(
			m_entityManager.get()->GetAllEntities().back().get(),
			m_entityManager.get()->GetAllEntities()[0].get()
		);

		// 트랜스폼은 
		// 시작위치, 스케일, 회전을 넣을 수 있어야 함
		m_transformManager.get()->AddComponent(
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID());

		GOE::Matrix4x4 tm = GOE::Matrix4x4::Identity();
		m_transformManager.get()->GetCurrentComponent()->SetLocalTM(tm);
		m_transformManager.get()->GetCurrentComponent()->SetScaleTM({ 0.1f, 0.1f, 0.1f });

		// 메쉬랜더러는 메쉬id를 전달하는 구간이 필요하다.
		m_meshRendererManager.get()->AddComponent(m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID());
		size_t meshPath = GOE::FileManager::GetHash("Ch03");

		m_meshRendererManager.get()->GetCurrentComponent()->SetMeshID(m_context->assetCore->GetMesh(meshPath)->GetID());
		m_meshRendererManager.get()->GetCurrentComponent()->SetMeshIndex(m_context->assetCore->GetMesh(meshPath)->GetMeshIndex());
		m_meshRendererManager.get()->GetCurrentComponent()->SetModelID(m_context->assetCore->GetMesh(meshPath)->GetModelID());
		m_meshRendererManager.get()->GetCurrentComponent()->SetName(m_context->assetCore->GetMesh(meshPath)->GetName());
		m_meshRendererManager.get()->GetCurrentComponent()->SetVisible(true);

		size_t textureid = GOE::FileManager::GetHash(GOE::FileManager::GetInstance().GetFullPath("Assets/textures/Ch03_1001_Diffuse.png"));
		m_materialManager.get()->AddComponent(
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID());
		m_materialManager.get()->GetCurrentComponent()->SetTextureID(textureid);

		size_t animationid = GOE::FileManager::GetHash("mixamo.com");
		m_animationUnitManager.get()->AddComponent(
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
			m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID());
		m_animationUnitManager.get()->GetCurrentComponent()->SetAnimationHash(animationid);
		m_animationUnitManager.get()->GetCurrentComponent()->SetAnimate(true);
	}


	/// 쿠라몬 1000개가 있으면 9-10FPS가 나온다.
	//for (int i = 0; i < 10; ++i)
	//{
	//	for (int j = 0; j < 10; ++j)
	//	{
	//		for (int k = 0; k < 10; ++k)
	//		{
	//			// 엔티티를 만들고
	//			std::string entityName = "쿠라몬_" + std::to_string(i) + "_" + std::to_string(j) + "_" + std::to_string(k);
	//			m_entityManager.get()->CreateEntity(entityName);

	//			// 트랜스폼은 
	//			// 시작위치, 스케일, 회전을 넣을 수 있어야 함
	//			m_transformManager.get()->AddComponent(
	//				m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
	//				m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID());

	//			GOE::Matrix4x4 tm = GOE::Matrix4x4::Identity();
	//			m_transformManager.get()->GetCurrentComponent()->SetLocalTM(tm);
	//			m_transformManager.get()->GetCurrentComponent()->SetScaleTM({ 0.1f, 0.1f, 0.1f });
	//			m_transformManager.get()->GetCurrentComponent()->SetPositionTM({ 15.0f * static_cast<float>(i), 15.0f * static_cast<float>(j), 15.0f * static_cast<float>(k) });

	//			// 메쉬랜더러는 메쉬id를 전달하는 구간이 필요하다.
	//			// 일단 컴포넌트 아이디와 entity 아이디를 동일하게 사용한다.
	//			m_meshRendererManager.get()->AddComponent(
	//				m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
	//				m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID());

	//			size_t meshid = GOE::FileManager::GetHash("chr629_0");
	//			m_meshRendererManager.get()->GetCurrentComponent()->SetMeshID(m_context->assetCore->GetMesh(meshid)->GetID());
	//			m_meshRendererManager.get()->GetCurrentComponent()->SetMeshIndex(m_context->assetCore->GetMesh(meshid)->GetMeshIndex());
	//			m_meshRendererManager.get()->GetCurrentComponent()->SetModelID(m_context->assetCore->GetMesh(meshid)->GetModelID());
	//			m_meshRendererManager.get()->GetCurrentComponent()->SetName(m_context->assetCore->GetMesh(meshid)->GetName());
	//			m_meshRendererManager.get()->GetCurrentComponent()->SetVisible(true);

	//			size_t textureid = GOE::FileManager::GetHash(GOE::FileManager::GetInstance().GetFullPath("Assets/textures/chr629a01.png"));
	//			m_materialManager.get()->AddComponent(
	//				m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
	//				m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID());
	//			m_materialManager.get()->GetCurrentComponent()->SetTextureID(textureid);

	//			size_t animationid = GOE::FileManager::GetHash("chr629_armature|chr629_ba01");
	//			m_animationUnitManager.get()->AddComponent(
	//				m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID(),
	//				m_entityManager.get()->GetAllEntities().back().get()->GetEntitiyID());
	//			m_animationUnitManager.get()->GetCurrentComponent()->SetAnimationHash(animationid);
	//			m_animationUnitManager.get()->GetCurrentComponent()->SetAnimate(true);
	//		}
	//	}
	//}
	
}