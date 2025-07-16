// GOE_AssetLoader/AssetCore.h
#include <memory> 
class AssetCoreImpl; // 내부 구현 클래스를 전방 선언

class AssetCore
{
public:
	AssetCore();
	~AssetCore(); // 소멸자는 cpp 파일에 구현해야 함

	// test 함수는 그대로 공개
	void test();

private:
	// 실제 구현을 가리키는 포인터만 멤버로 가짐
	std::unique_ptr<AssetCoreImpl> m_pimpl;
};