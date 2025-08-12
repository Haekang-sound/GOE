#pragma once  
#include <memory>
#include <Windows.h>

namespace GOE
{
	class IEngine;
}

/// <summary>
/// 프로그램의 최전방 Application
/// 
/// </summary>
class Application
{
public:
	Application(HINSTANCE hInst, int nCmdShow);
	~Application();

public:
	void Initialize();
	int Run();

private:
	std::unique_ptr<GOE::IEngine> m_engine;
};
