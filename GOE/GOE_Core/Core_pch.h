#pragma once
///
/// STL의 컴파일 부담을 없애려면
/// 미리컴파일된 헤더를 사용하는게 좋을것
/// 같다고 판단해서 만들게된 pch 파일입니다.
/// 
/// ohk 2025.07.21
/// 
#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <comdef.h>
#include <windows.h>
#include <stdexcept>
#include <functional>
#include <vector>


inline void ThrowIfFailed(const HRESULT& hr)
{
	if (FAILED(hr))
	{
		throw std::runtime_error("HRESULT failed!");
	}
}

using Microsoft::WRL::ComPtr;
using namespace Microsoft::WRL;