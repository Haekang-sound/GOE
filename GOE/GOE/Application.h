#pragma once
#include "framework.h"
#include "resource.h"

class Application
{
private:
	HWND m_hWnd;
	HINSTANCE m_hInst;
	MSG m_msg;

public: 
	MSG* GetMsg() { return &m_msg; };


public:
	ATOM                MyRegisterClass(HINSTANCE hInstance);
	BOOL                InitInstance(HINSTANCE hInstance, int nCmdShow);
	static LRESULT CALLBACK    WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public: 
	void Initialize();
	void Update();
	void Render();
};

