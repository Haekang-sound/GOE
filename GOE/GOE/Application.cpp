#include "Application.h"
#include "../GOERender/GOERenderer.h"  

Application::Application(GOE::WinDesc info)
	: m_hInst(info.hInstance), m_nCmdShow(info.nCmdShow),
	m_name(info.name), m_windowSize({ static_cast<SHORT>(info.width), static_cast<SHORT>(info.height) }),
	m_renderer(nullptr), m_hWnd(nullptr), m_msg({})
{ }

void Application::Initialize()
{
	InitInstance(); // 인스턴스 초기화 함수 호출

	/// 나중에 엔진으로 편입될예정 반드시 지워야해
	m_renderer = std::make_unique<GOERenderer>(m_hWnd); // GOERenderer 인스턴스 생성
	m_renderer->OnInit(); // GOERenderer 초기화
}

int Application::Run()
{
	while (true)
	{
		// 메시지가 있으면 처리
		if (PeekMessage(&m_msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (m_msg.message == WM_QUIT)
				break;

			TranslateMessage(&m_msg);
			DispatchMessage(&m_msg);
		}

		m_renderer->OnRender(); // 렌더링 호출

	}

	return 0;
}

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
	wcex.lpszClassName = m_name.c_str();                 // 이 윈도우 클래스의 이름
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL)); // 작은 아이콘 (타이틀 바에 표시)
	return RegisterClassExW(&wcex);                     // 설정한 윈도우 클래스를 운영체제에 등록
}

BOOL Application::InitInstance()
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
		nullptr);				// 추가 매개변수 (없으면 nullptr)

	if (!m_hWnd)
	{
		return FALSE;
	}
	
	ShowWindow(m_hWnd, m_nCmdShow);
	UpdateWindow(m_hWnd);

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


