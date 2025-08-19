#pragma once
#include<memory>
#include <Windows.h>
#include "IEngine.h"

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
		
	public:
		void Initialize() override;

		void OnUpdate(double dTime) override;

		void BeginRender() override;
		void OnRender() override;
		void EndRender() override;

		void Release();

	private:
		void DebugUpdate();
		void InputUpdate();

	private:
		std::unique_ptr<GOE::EngineContext> m_context;

		HINSTANCE m_hInst = 0;
		int m_nCmdShow = 0;

	private:
		std::unique_ptr<Window> m_winCore;
		std::unique_ptr<GOE::ID3DRenderer> m_renderer;
		std::unique_ptr<EditorCore> m_editor;
		std::unique_ptr<AssetCore> m_assetCore;

	private:
		std::unique_ptr<SceneManager> m_sceneManager;
	};
}


