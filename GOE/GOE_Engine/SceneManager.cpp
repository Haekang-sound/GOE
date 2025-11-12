#include "Engine_pch.h"
#include "SceneManager.h"
#include "Scene.h"

SceneManager::SceneManager()
	:currentScene(nullptr), m_context(nullptr)
{
	currentScene = std::make_unique<Scene>();
}

SceneManager::~SceneManager() = default;

void SceneManager::Initialize(GOE::EngineContext* context)
{
	currentScene.get()->Initialize(context);
}

/// <summary>
///  시스템의 업데이트가 일어나야한다.
/// </summary>
/// <param name="dTime"></param>
void SceneManager::OnUpdate(double dTime)
{
	currentScene.get()->OnUpdate(dTime);
}

void SceneManager::BeginRender()
{

}

void SceneManager::OnRender()
{

}

void SceneManager::EndRender()
{

}

void SceneManager::Release()
{

}

void SceneManager::InputUpdate()
{

}

void SceneManager::DebugUpdate(double dTime)
{
	currentScene.get()->DebugUpdate(dTime);
}