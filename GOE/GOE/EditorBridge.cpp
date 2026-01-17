#include "EditorBridge.h"

#include "../GOE_Engine/Engine.h"  
// 필요한 엔진 헤더들
#include "../GOE_Engine/SceneManager.h"
#include "../GOE_Engine/Scene.h"
#include "../GOE_Engine/EntityManager.h"
#include "../GOE_Engine/Entity.h"

Editor::EditorBridge::EditorBridge(SceneManager* _sceneManager)
	: m_sceneManager(_sceneManager)
{
}

Editor::EditorBridge::~EditorBridge() = default;

/// <summary>
///  엔티티 정보를 실시간으로 갱신합니다.
/// </summary>
/// <returns></returns>
std::vector<Editor::EntityInfo>& Editor::EditorBridge::GetAllEntities()
{
	// 매번 채우고 전달하기 때문에 초기화
	m_infos.clear();

	if (!m_sceneManager) return m_infos;

	// 1. 현재 씬 가져오기
	m_currentScene = m_sceneManager->GetCurrentScene();

	// 2. 엔티티 가져오기
	const auto& entities = m_currentScene->GetEntityManager()->GetAllEntities();

	// 1. 부모가 없는 애들을 채운다. -> 제일 부모다.
	for (const auto& entity : entities)
	{
		if (!entity.get()->HasParent())
		{
			m_infos.emplace_back(
				entity.get()->GetEntitiyID(),
				entity.get()->GetName());
		}
	}

	AddChildrenInfos(m_infos);

	return m_infos;
}

void Editor::EditorBridge::OnEntitySelected(size_t entityID)
{
	std::string msg = "[EditorBridge] Entity Selected: " + std::to_string(entityID) + "\n";
	OutputDebugStringA(msg.c_str());
}

void Editor::EditorBridge::AddChildrenInfos(std::vector<EntityInfo>& _vec)
{
	const auto& entities = m_currentScene->GetEntityManager()->GetAllEntities();
	
	// 최상위 부모 순회
	for (auto& parent : _vec)
	{
		// 맵에 접근해서 엔티티index를 찾자.
		// id - index 맵
		auto& Map = m_currentScene->GetEntityManager()->GetEntityMap();
		auto it = Map.find(parent.id);
		if (it == Map.end()) continue;
		// 인덱스를 찾았다.
		auto idx = it->second;

		// 자식이 없으면 채울게 없잖아
		if (!entities[idx].get()->HasChildren()) continue;

		// 부모의 자식벡터를 순회한다
		for (auto& childID : entities[idx].get()->GetChildern())
		{
			auto it = Map.find(childID);
			if (it == Map.end()) continue;
			// 인덱스를 찾았다.
			auto childIdx = it->second;
			// 부모의 info에 자식의 info를 채운다.
			parent.children.emplace_back(
				entities[childIdx].get()->GetEntitiyID(),
				entities[childIdx].get()->GetName());
		}

		AddChildrenInfos(parent.children);
	}
}
