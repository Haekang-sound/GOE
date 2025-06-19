#pragma once  
#include "framework.h"  
#include "resource.h"  
//#pragma comment(lib, "GOERender.lib")
#include "../GOERender/GOERenderer.h"  

class Application  
{  
	public:  
	Application() : m_hInst(nullptr), m_renderer(nullptr) {}  
	~Application() { delete m_renderer; }	
private:  
	static HWND m_hWnd;  
	HINSTANCE m_hInst;  
	MSG m_msg;

public:  
	MSG* GetMsg() { return &m_msg; };  
	GOERenderer* m_renderer;
public:  
	ATOM MyRegisterClass(HINSTANCE hInstance);  
	BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);  
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);  
	HWND GetHWND() { return m_hWnd; }
public:  
	void Initialize();  
	void Update();  
	void Render();  
};
