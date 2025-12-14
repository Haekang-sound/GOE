#include "EditorBridge.h"
#include "../GOE_Editor/IEditorBridge.h"

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

std::vector<Editor::EntityInfo> Editor::EditorBridge::GetAllEntities()
{
	std::vector<Editor::EntityInfo> result;
	if (!m_sceneManager) return result;

	// 1. 현재 씬 가져오기
	Scene* currentScene = m_sceneManager->GetCurrentScene();

	// 2. 엔티티 가져오기
	const auto& entities = currentScene->GetEntityManager()->GetAllEntities();
	for (const auto& entity : entities)
	{
		Editor::EntityInfo info;
		info.id = entity.get()->GetEntitiyID();
		info.name = entity.get()->GetName();

		result.push_back(info);
	}

	return result;
}

void Editor::EditorBridge::OnEntitySelected(size_t entityID)
{
	std::string msg = "[EditorBridge] Entity Selected: " + std::to_string(entityID) + "\n";
	OutputDebugStringA(msg.c_str());
}
