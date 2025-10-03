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
#include <iostream> 
#include <unordered_map>

#include "CoreMath.h"
#include "FileManager.h"

// 모델정보
#include "Model.h"
#include "Mesh.h"
#include "Node.h"
#include "Bone.h"
#include "Texture.h"
#include "Animation.h"
#include "BoneAnimation.h"

using Microsoft::WRL::ComPtr;
using namespace Microsoft::WRL;

inline void ThrowIfFailed(const HRESULT& hr)
{
	if (FAILED(hr))
	{
		throw std::runtime_error("HRESULT failed!");
	}
}

