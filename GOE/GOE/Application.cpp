#include "Application.h"
#include "../GOE_Engine/IEngine.h"
#include "../GOE_Engine/Engine.h"
#include <chrono>

Application::Application(HINSTANCE hInst, int nCmdShow)
	: m_engine(nullptr)
{
	m_engine = std::make_unique<GOE::Engine>(hInst, nCmdShow);
}

Application::~Application(){}

void Application::Initialize()
{
	m_engine.get()->Initialize();
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

		// 델타 타임 계산
		auto currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		// 게임 로직 실행
		m_engine->OnUpdate(deltaTime.count()); // 실제 경과 시간을 전달

		m_engine->BeginRender();
		m_engine->OnRender();
		m_engine->EndRender();
	}
	return 0;
}