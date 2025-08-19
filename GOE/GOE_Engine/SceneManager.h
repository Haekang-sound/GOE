#pragma once
#include <memory>

namespace GOE
{
	struct EngeinContext;
}
class Scene;

/// <summary>
/// 씬을 받아서 돌리는 씬매니저
/// </summary>
class SceneManager
{
public:
	SceneManager();
	~SceneManager();

public:
	void Initialize(GOE::EngineContext* context);

	void OnUpdate(double dTime);

	void BeginRender();
	void OnRender();
	void EndRender();

	void Release();

	void InputUpdate();
	void DebugUpdate();

private:
	std::unique_ptr<Scene> currentScene;
	GOE::EngeinContext* m_context;
private:
};

