#include "Application.h"

ATOM Application::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;                       // ������ Ŭ���� ������ ��� ����ü (Ȯ��� ����)
	wcex.cbSize = sizeof(WNDCLASSEX);       // ����ü�� ũ�� ���� (�ʼ�)
	wcex.style = CS_HREDRAW | CS_VREDRAW;   // â ũ�� ���� �� ��ü�� �ٽ� �׸����� ����
	wcex.lpfnWndProc = WndProc;             // ������ �޽����� ó���� �ݹ� �Լ� (������ ���ν���)
	wcex.cbClsExtra = 0;                    // Ŭ���� ���� �޸� (������� ������ 0)
	wcex.cbWndExtra = 0;                    // ������ �ν��Ͻ� ���� �޸� (������� ������ 0)
	wcex.hInstance = hInstance;             // ���ø����̼� �ν��Ͻ� �ڵ�
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GOE)); // ū ������ �ε�S
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);      // �⺻ ���콺 Ŀ�� ���� (ȭ��ǥ)
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);    // ��� �귯�� (�⺻ â ����)
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GOE);      // ���ҽ��� ���ǵ� �޴� �̸� ����
	wcex.lpszClassName = L"GOE";                 // �� ������ Ŭ������ �̸�
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL)); // ���� ������ (Ÿ��Ʋ �ٿ� ǥ��)
	return RegisterClassExW(&wcex);                     // ������ ������ Ŭ������ �ü���� ���
}

BOOL Application::InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	m_hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.

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
			PostQuitMessage(0); // ���ø����̼� ���� ��û
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam); // �⺻ ó�� ����
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
