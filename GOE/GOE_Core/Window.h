#pragma once
#include <Windows.h>
#include <string>
#include <functional>

class Window
{
public:
	Window(const std::wstring& title, int width, int height, HINSTANCE hInstance, int nCmdShow);
	~Window();

	// Application이 호출할 간단한 인터페이스
	bool InitInstance();
	ATOM MyRegisterClass(HINSTANCE hInstance);
	bool ProcessMessages(); // 메시지 루프를 처리하고, 종료 여부를 반환

public:
	inline HWND GetHWND() const { return m_hWnd; }
	inline void SetExternalMsgHandler(std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> handler)
	{
		m_imguiCallback = handler;
	};

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> m_imguiCallback;

	const std::wstring m_name;
	HWND m_hWnd = nullptr;
	HINSTANCE m_hInst = nullptr;
	MSG m_msg;
	int m_nCmdShow;
	COORD m_windowSize;
};