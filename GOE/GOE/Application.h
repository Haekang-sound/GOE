#pragma once  
#include <memory>
#include <Windows.h>

class Window;
class GOERenderer;
class EditorCore;
class AssetCore;

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
	std::unique_ptr<Window> m_winCore;
	std::unique_ptr<GOERenderer> m_renderer;
	std::unique_ptr<EditorCore> m_editor;
	std::unique_ptr<AssetCore> m_assetCore;
};
