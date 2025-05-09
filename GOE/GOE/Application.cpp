#include "Application.h"

ATOM Application::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;                       // 윈도우 클래스 정보를 담는 구조체 (확장된 버전)
	wcex.cbSize = sizeof(WNDCLASSEX);       // 구조체의 크기 설정 (필수)
	wcex.style = CS_HREDRAW | CS_VREDRAW;   // 창 크기 변경 시 전체를 다시 그리도록 설정
	wcex.lpfnWndProc = WndProc;             // 윈도우 메시지를 처리할 콜백 함수 (윈도우 프로시저)
	wcex.cbClsExtra = 0;                    // 클래스 여분 메모리 (사용하지 않으면 0)
	wcex.cbWndExtra = 0;                    // 윈도우 인스턴스 여분 메모리 (사용하지 않으면 0)
	wcex.hInstance = hInstance;             // 애플리케이션 인스턴스 핸들
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GOE)); // 큰 아이콘 로드S
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);      // 기본 마우스 커서 지정 (화살표)
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);    // 배경 브러시 (기본 창 배경색)
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GOE);      // 리소스에 정의된 메뉴 이름 지정
	wcex.lpszClassName = L"GOE";                 // 이 윈도우 클래스의 이름
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL)); // 작은 아이콘 (타이틀 바에 표시)
	return RegisterClassExW(&wcex);                     // 설정한 윈도우 클래스를 운영체제에 등록
}

BOOL Application::InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	m_hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	MyRegisterClass(m_hInst);

	HWND hWnd = CreateWindowW(L"GOE", L"GOE", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT Application::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

		case WM_DESTROY:
			PostQuitMessage(0); // 애플리케이션 종료 요청
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam); // 기본 처리 위임
	}

	return 0;
}

void Application::Initialize()
{
}

void Application::Update()
{
}

void Application::Render()
{
}
