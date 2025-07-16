// GOE_AssetLoader/AssetCore.cpp

#include "AssetCore.h"
#include <assimp/Importer.hpp> // assimp 헤더는 cpp 파일에서만 포함!
#include <iostream>

// 헤더에서 전방 선언했던 내부 클래스를 여기서 실제로 정의
class AssetCoreImpl
{
public:
	void test()
	{
		Assimp::Importer a;
		//std::cout << "Assimp version: " << a.GetVersionMajor() << "." << a.GetVersionMinor() << std::endl;
		std::cout << "Error string: " << a.GetErrorString() << std::endl;
	}
};

// --- AssetCore의 멤버 함수 구현 ---

AssetCore::AssetCore()
	: m_pimpl(std::make_unique<AssetCoreImpl>()) // 생성자에서 내부 클래스 생성
{
}

AssetCore::~AssetCore() = default; // 소멸자 구현

void AssetCore::test()
{
	// 실제 작업은 내부 클래스에게 위임
	m_pimpl->test();
}