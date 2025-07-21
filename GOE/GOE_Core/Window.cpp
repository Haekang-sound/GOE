#include "Core_pch.h"
#include "Window.h"
#include "../GOE/Resource.h"  
Window::Window(const std::wstring& title, int width, int height, HINSTANCE hInstance, int nCmdShow)
    : m_name(title), m_hInst(hInstance), m_msg{}, m_nCmdShow(nCmdShow)
{  
    m_windowSize.X = width;  
    m_windowSize.Y = height;  
}

Window::~Window()
{}


bool Window::InitInstance()
{
	MyRegisterClass(m_hInst);

	m_hWnd = CreateWindowW(
		m_name.c_str(),			// 윈도우 클래스 이름
		m_name.c_str(),			// 윈도우 제목
		WS_OVERLAPPEDWINDOW,	// 윈도우 스타일 (오버랩된 창)
		CW_USEDEFAULT,			// x 위치 (기본값)
		0,						// y 위치 (기본값)
		m_windowSize.X,			// 너비 (기본값)
		m_windowSize.Y,			// 높이 (기본값)
		nullptr,				// 부모 윈도우 핸들 (없으면 nullptr)
		nullptr,				// 메뉴 핸들 (없으면 nullptr)
		m_hInst,				// 애플리케이션 인스턴스 핸들
		this);					// this를 넣어서 나중에 사용할예정

	if (!m_hWnd)
	{
		return false;
	}

	ShowWindow(m_hWnd, m_nCmdShow);
	UpdateWindow(m_hWnd);

	return true;
}
ATOM Window::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;                       // 윈도우 클래스 정보를 담는 구조체 (확장된 버전)
	wcex.cbSize = sizeof(WNDCLASSEX);       // 구조체의 크기 설정 (필수)
	wcex.style = CS_HREDRAW | CS_VREDRAW;   // 창 크기 변경 시 전체를 다시 그리도록 설정
	wcex.lpfnWndProc = WndProc;             // 윈도우 메시지를 처리할 콜백 함수 (윈도우 프로시저)
	wcex.cbClsExtra = 0;                    // 클래스 여분 메모리 (사용하지 않으면 0)
	wcex.cbWndExtra = 0;                    // 윈도우 인스턴스 여분 메모리 (사용하지 않으면 0)
	wcex.hInstance = m_hInst;				// 애플리케이션 인스턴스 핸들
	wcex.hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_GOE)); // 큰 아이콘 로드S
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);      // 기본 마우스 커서 지정 (화살표)
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);    // 배경 브러시 (기본 창 배경색)
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GOE);      // 리소스에 정의된 메뉴 이름 지정
	wcex.lpszClassName = m_name.c_str();                 // 이 윈도우 클래스의 이름
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL)); // 작은 아이콘 (타이틀 바에 표시)
	return RegisterClassExW(&wcex);                     // 설정한 윈도우 클래스를 운영체제에 등록
}

bool Window::ProcessMessages()
{
	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return false; // 종료 신호

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true; // 계속 진행
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window* pWindow = nullptr;

	if (message == WM_CREATE)
	{
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		pWindow = reinterpret_cast<Window*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pWindow);
	}
	else
	{
		pWindow = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	// pWindow 포인터가 유효할 때만 내부 로직을 처리합니다.
	if (pWindow)
	{
		// 1. ImGui 콜백이 메시지를 처리했는지 먼저 확인합니다.
		if (pWindow->m_imguiCallback && pWindow->m_imguiCallback(hWnd, message, wParam, lParam))
		{
			return true; // ImGui가 처리했으면 여기서 종료
		}

		// 2. 직접 처리할 메시지가 있는지 확인합니다.
		switch (message)
		{
			case WM_DESTROY:
				PostQuitMessage(0);
				return 0; // 처리가 끝났으므로 0을 반환하고 함수 종료
		}
	}

	// pWindow가 아직 유효하지 않거나, 위에서 처리되지 않은 모든 메시지는
	// 기본 처리기(DefWindowProc)에 맡깁니다.
	return DefWindowProc(hWnd, message, wParam, lParam);
}