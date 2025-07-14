#include "Application.h"

/// <summary>
/// 어플리케이션의 EntryPoint
/// </summary>
/// <param name="hInstance"></param>
/// <param name="hPrevInstance"></param>
/// <param name="lpCmdLine"></param>
/// <param name="nCmdShow"></param>
/// <returns></returns>
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,	_In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	std::unique_ptr<Application> app = std::make_unique<Application>(hInstance, nCmdShow);
	app->Initialize(); // 애플리케이션 초기화
	return app->Run();
}
