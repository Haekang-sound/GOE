#pragma once
#include "framework.h"  

namespace GOE
{
	struct WinDesc
	{
		std::wstring name;
		HINSTANCE hInstance;
		int nCmdShow;
		size_t height;
		size_t width;
	};
}
