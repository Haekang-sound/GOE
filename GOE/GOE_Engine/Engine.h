#pragma once
#include<memory>
#include <Windows.h>
#include "IEngine.h"
#include <string>

namespace GOE
{
	class ID3DRenderer;
	struct EngineContext;
}
class Window;
class EditorCore;
class AssetCore;
class SceneManager;


/// <summary>
/// 기능들이 만들어질 엔진
/// </summary>
namespace GOE
{
	class Engine : public IEngine
	{
	public:
		Engine(HINSTANCE hInst, int nCmdShow);
		~Engine();

	private:
		HINSTANCE m_hInst = 0;
		int m_nCmdShow = 0;
		std::unique_ptr<GOE::EngineContext> m_context;

	private:
		std::unique_ptr<Window> m_winCore;
		std::unique_ptr<GOE::ID3DRenderer> m_renderer;
		std::unique_ptr<EditorCore> m_editor;
		std::unique_ptr<AssetCore> m_assetCore;

	private:
		std::unique_ptr<SceneManager> m_sceneManager;

	private:
		double m_fpsTimer = 0.0;
		int m_frameCount = 0;
		size_t DestroyWndID = 0;	// 함수id

	public:
		void Initialize() override;
		void OnUpdate() override;
		void DebugUpdate(double dTime);
		void BeginRender() override;
		void OnRender() override;
		void EndRender() override;
		void Release();
	};
}


