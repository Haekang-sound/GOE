// GOE.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#include "Application.h"




int APIENTRY wWinMain(_In_ HINSTANCE hInstance,	_In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	GOE::WinDesc winInfo = { L"GOE", hInstance, nCmdShow, 1920, 1080 };
	std::unique_ptr<Application> app = std::make_unique<Application>(winInfo);
	app->Initialize(); // 애플리케이션 초기화
	return app->Run();
}
