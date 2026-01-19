#pragma once
#include "IEditorBridge.h"
#include <vector>
class SceneManager;
class Entity;
class Scene;
namespace Editor
{
	class EditorBridge : public IEditorBridge
	{
	public:
		EditorBridge(SceneManager* _sceneManager);
		virtual ~EditorBridge();

	protected:
		SceneManager* m_sceneManager = nullptr;
		std::vector<Editor::EntityInfo> m_infos;
		Scene* m_currentScene = nullptr;

	public:
		// [인터페이스 구현]
		virtual std::vector<EntityInfo>& GetAllEntities() override;
		virtual void OnEntitySelected(size_t entityID) override;

	protected:
		void AddChildrenInfos(std::vector<EntityInfo>& _vec);
	};
}

