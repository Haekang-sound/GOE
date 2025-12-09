#include "Core_pch.h"
#include "InputManager.h"
GOE::InputManager::InputManager()
	:m_downListeners{}, m_upListeners{}, m_hWnd{}
{
	m_keyStates.resize(256, KeyState::NONE);
}

GOE::InputManager::~InputManager() = default;

void GOE::InputManager::Initialize(HWND hwnd)
{
	m_hWnd = hwnd;
}

void GOE::InputManager::Update()
{
	/// 마우스 로직
	POINT pt;
	GetCursorPos(&pt); // 스크린 기준 좌표 가져오기

	if (m_hWnd)
	{
		ScreenToClient(m_hWnd, &pt); // 윈도우 기준 좌표로 변환
	}

	m_mousePos = pt;

	// 델타값 계산 (현재 - 이전)
	m_mouseDelta.x = m_mousePos.x - m_prevMousePos.x;
	m_mouseDelta.y = m_mousePos.y - m_prevMousePos.y;

	m_prevMousePos = m_mousePos; // 현재를 이전으로 저장
	
	/// 키보드
	// 가상키 목록전체를 순회합니다.
	for (int i = 0; i < 256; ++i)
	{
		// 해당키가 눌려있는지 검사한다.
		if (GetAsyncKeyState(i) & 0x8000)
		{
			// 키가 처음 눌렸다면
			if (m_keyStates[i] == KeyState::NONE || m_keyStates[i] == KeyState::UP)
			{
				// down 상태로 전환한다.
				m_keyStates[i] = KeyState::DOWN;
				DispatchDownEvent(i);
			}
			else
			{
				// 눌려있는데 down이면 Pressed
				m_keyStates[i] = KeyState::PRESSED;
			}
		}
		else // 키가 눌려있지 않은경우 
		{
			// DONW||PRESSED였을 경우
			if (m_keyStates[i] == KeyState::DOWN || m_keyStates[i] == KeyState::PRESSED)
			{
				// UP으로 전환한다.
				m_keyStates[i] = KeyState::UP;
				DispatchUpEvent(i);
			}
			else // 그렇지 않다면 NONE
			{
				m_keyStates[i] = KeyState::NONE;
			}
		}
	}
}

/// <summary>
/// 버튼상태를 반환합니다.
/// </summary>
/// <param name="key">키</param>
/// <returns></returns>
bool GOE::InputManager::GetButtonDown(int key)
{
	// 입력받은 값이 범위를 벗어나면 return false
	if (key < 0 || key >= 256) return false;
	return m_keyStates[key] == KeyState::DOWN;
}

bool GOE::InputManager::GetButton(int key)
{
	// 입력받은 값이 범위를 벗어나면 return false
	if (key < 0 || key >= 256) return false;
	return m_keyStates[key] == KeyState::PRESSED;
}

bool GOE::InputManager::GetButtonUp(int key)
{
	// 입력받은 값이 범위를 벗어나면 return false
	if (key < 0 || key >= 256) return false;
	return m_keyStates[key] == KeyState::UP;
}

/// <summary>
/// key에 함수를 바인딩 합니다. 
/// 바인딩후 함수의 id를 반환합니다.
/// 함수를 바인딩 한 클래스는 반환된 id를 관리해야합니다.
/// 
/// </summary>
/// <param name="key">키</param>
/// <param name="state">상태</param>
/// <param name="listener">바인딩 함수</param>
/// <returns>ID</returns>
GOE::EventID GOE::InputManager::BindAction(int key, KeyState state, std::function<void()> fnc)
{
	size_t id = GOE::FileManager::GetInstance().GetRendomHash();

	if (state == KeyState::DOWN)
	{
		m_downListeners[key].emplace_back(id, fnc);
	}
	else if (state == KeyState::UP)
	{
		m_upListeners[key].emplace_back(id, fnc);
	}

	return id;
}

/// <summary>
/// 해당 Key가 UP상태일 경우
/// 바인딩된 함수들을 순서대로 실행합니다.
/// </summary>
/// <param name="key">키</param>
void GOE::InputManager::DispatchUpEvent(int key)
{
	// m_upListeners만 확인
	auto it = m_upListeners.find(key);
	if (it != m_upListeners.end())
	{
		for (const auto& func : it->second)
		{
			// 액션이 존재하면 실행한다.
			if (func.action)
			{
				func.action();
			}
		}
	}
}

/// <summary>
/// 해당 Key가 DOWN상태일 경우
/// 바인딩된 함수들을 순서대로 실행합니다.
/// </summary>
/// <param name="key">키</param>
void GOE::InputManager::DispatchDownEvent(int key)
{
	// m_upListeners만 확인
	auto it = m_downListeners.find(key);
	if (it != m_downListeners.end())
	{
		for (const auto& func : it->second)
		{
			// 액션이 존재하면 실행한다.
			if (func.action)
			{
				func.action();
			}
		}
	}
}

