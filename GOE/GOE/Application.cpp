#include "Application.h"
#include "EditorBridge.h"
#include "../GOE_Engine/Engine.h"
#include "../GOE_Editor/EditorCore.h"
#include "../GOE_Core/TimeManager.h"

Application::Application(HINSTANCE hInst, int nCmdShow)
	: m_engine(nullptr)
{
	m_engine = std::make_unique<GOE::Engine>(hInst, nCmdShow);

}

Application::~Application(){}

void Application::Initialize()
{
	m_engine.get()->Initialize();

	m_editorCore = std::make_unique<Editor::EditorCore>(m_engine->GetHWND());
	m_editorCore->Initialize(m_engine.get()->GetUIInfo());

	m_editorBridge = std::make_unique<Editor::EditorBridge>(m_engine.get()->GetSceneManager());
	m_editorCore->SetBridge(m_editorBridge.get());

}

int Application::Run()
{
	MSG msg = {};
	bool isRunning = true;

	// 시간 측정을 위한 변수
	auto lastTime = std::chrono::high_resolution_clock::now();

	while (isRunning)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				isRunning = false; // 메인 루프를 탈출하도록 플래그 설정
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// 게임 로직 실행
		m_engine->OnUpdate(); // 실제 경과 시간을 전달

		m_engine->BeginRender();
		m_engine->OnRender();

		m_editorCore->OnUpdate(GOE::TimeManager::GetInstance().GetDeltaTime());
		m_editorCore->OnRender(m_engine.get()->GetUILoopInfo());

		m_engine->EndRender();
	}
	return 0;
}