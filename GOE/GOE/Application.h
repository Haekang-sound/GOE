#pragma once  

#include "resource.h"  
#include "../GOE/Infomation.h"
#include "../GOERender/ID3DRenderer.h"

class ID3DRenderer;

/// <summary>
/// 프로그램의 최전방 Application
/// 
/// </summary>
class Application
{
public:
	Application(GOE::WinDesc info);
	~Application() {}

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
	
	std::unique_ptr<ID3DRenderer> m_renderer;

public:
	ATOM MyRegisterClass(HINSTANCE hInstance);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL InitInstance();
};
