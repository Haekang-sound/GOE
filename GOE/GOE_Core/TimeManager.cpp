#include "Core_pch.h"
#include "TimeManager.h"

GOE::TimeManager::TimeManager()
	: m_deltaTime(0.0)
{
}

GOE::TimeManager::~TimeManager() = default;

/// <summary>
/// 타임매니저 초기화
/// </summary>
void GOE::TimeManager::Initialize()
{
	m_prevTime = std::chrono::high_resolution_clock::now();
}

/// <summary>
/// 타임매니저 업데이트
/// </summary>
void GOE::TimeManager::Update()
{
	// 현재시간을 입력받는다.
	auto currentTime = std::chrono::high_resolution_clock::now();
	// 현재시간에서 이전시간을 빼서 프레임간격을 구한다.
	std::chrono::duration<double> duration = currentTime - m_prevTime;

	// 현재시간과 프레임간격을 저장합니다.
	m_prevTime = currentTime;
	m_deltaTime = duration.count();
}
