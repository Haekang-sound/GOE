#pragma once
#include"../GOE_Core/SingletonBase.h"
#include <functional>
#include <queue>

/// <summary>
/// 델리게이트 방식으로 
/// 디버깅정보를 띄워주는 DebugManager
/// 
/// 싱글턴으로 사용합니다.
/// 
/// ohk	2025.07.11
/// </summary>
class DebugManager : public SingletonBase<DebugManager>
{
public:
	DebugManager();
	~DebugManager();

public: 
	// update에서 정보를 받아오고 나서 실행하야 하니까 
	// 일반적인 업데이트와 실행순서가 구분되어야 한다.
	// lateupdate와도 완전히 구분, 게임로직에 영향을 주면 안된다.
	void OnDebugUpdate();
	void OnRender();

public: 
	// funtional Queue 를 이용해서 함수들을 전달받고 
	// 순서대로 재생 pop
	inline void PushDebugData(std::function<void()> debugData) { m_debugQueue.push(debugData); }

private:
	std::queue<std::function<void()>> m_debugQueue = {};

}; 

