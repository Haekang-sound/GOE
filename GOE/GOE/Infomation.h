#pragma once
#include "framework.h"  


namespace GOE
{
	struct WinDesc
	{
		std::wstring name;
		HINSTANCE hInstance;
		int nCmdShow;
		size_t width;
		size_t height;
	};

}
