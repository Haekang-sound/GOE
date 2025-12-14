#pragma once
#include "IEditorBridge.h"
class SceneManager;

namespace Editor
{
	class EditorBridge : public IEditorBridge
	{
	public:
		EditorBridge(SceneManager* _sceneManager);
		virtual ~EditorBridge();

	protected:
		SceneManager* m_sceneManager = nullptr;

	public:
		// [인터페이스 구현]
		virtual std::vector<Editor::EntityInfo> GetAllEntities() override;
		virtual void OnEntitySelected(size_t entityID) override;
	};
}

