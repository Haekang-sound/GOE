#pragma once  
#include <memory>
#include <Windows.h>

namespace GOE
{
	class IEngine;
}
namespace Editor
{
	class EditorBridge;
	class EditorCore;
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
	std::unique_ptr<Editor::EditorCore> m_editorCore;
	std::unique_ptr<Editor::EditorBridge> m_editorBridge;
};
