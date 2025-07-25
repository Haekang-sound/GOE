#pragma once
///
/// STL의 컴파일 부담을 없애려면
/// 미리컴파일된 헤더를 사용하는게 좋을것
/// 같다고 판단해서 만들게된 pch 파일입니다.
/// 
/// ohk 2025.07.21
#include <../GOE_Core/Core_pch.h>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/LogStream.hpp>
#include <assimp/DefaultLogger.hpp>