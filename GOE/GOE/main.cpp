// GOE.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//



#include "Application.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{

	std::unique_ptr<Application> app = std::make_unique<Application>();

	if (!app->InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	while (true)
	{
		// 메시지가 있으면 처리
		if (PeekMessage(app->GetMsg(), nullptr, 0, 0, PM_REMOVE))
		{
			if (app->GetMsg()->message == WM_QUIT)
				break;

			TranslateMessage(app->GetMsg());
			DispatchMessage(app->GetMsg());
		}

		app->Update();
		app->Render();
	}

	return 0;
}