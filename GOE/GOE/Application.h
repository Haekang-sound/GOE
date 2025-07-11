#pragma once  
#include "resource.h"  
#include "../GOE/Infomation.h"
#include "../GOE_Render/ID3DRenderer.h"
#include "../GOE_Editor/EditorCore.h"

/// <summary>
/// 프로그램의 최전방 Application
/// 
/// </summary>
class Application
{
public:
	Application(GOE::WinDesc info);
	~Application();

public:
	void Initialize();
	int Run();

private:
	std::wstring m_name;
	HWND m_hWnd;
	HINSTANCE m_hInst;
	MSG m_msg;
	int m_nCmdShow;
	COORD m_windowSize;

private:
	std::unique_ptr<ID3DRenderer> m_renderer;
	std::unique_ptr<EditorCore> m_editor;

public:
	ATOM MyRegisterClass(HINSTANCE hInstance);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL InitInstance();
};
