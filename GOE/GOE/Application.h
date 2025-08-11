#pragma once  
#include <memory>
#include <Windows.h>

namespace GOE
{
	class ID3DRenderer;
}
class Window;
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
	std::unique_ptr<GOE::ID3DRenderer> m_renderer;
	std::unique_ptr<EditorCore> m_editor;
	std::unique_ptr<AssetCore> m_assetCore;
};
