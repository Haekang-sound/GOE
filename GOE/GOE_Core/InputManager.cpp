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
	m_screenMousePos = pt;

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
/// 버튼 상태를 반환합니다.
/// 
/// </summary>
/// <param name="key">키</param>
/// <returns></returns>
bool GOE::InputManager::GetButtonDown(KEY key)
{
	// 입력받은 값이 범위를 벗어나면 return false
	if (key < 0 || key >= 256) return false;
	return m_keyStates[key] == KeyState::DOWN;
}

bool GOE::InputManager::GetButton(KEY key)
{
	// 입력받은 값이 범위를 벗어나면 return false
	if (key < 0 || key >= 256) return false;
	return m_keyStates[key] == KeyState::PRESSED;
}

bool GOE::InputManager::GetButtonUp(KEY key)
{
	// 입력받은 값이 범위를 벗어나면 return false
	if (key < 0 || key >= 256) return false;
	return m_keyStates[key] == KeyState::UP;
}

bool GOE::InputManager::GetMouseButtonDown(MouseButton button)
{
	return GetButtonDown(MouseButtonToKey(button));
}

bool GOE::InputManager::GetMouseButton(MouseButton button)
{
	return GetButton(MouseButtonToKey(button));
}

bool GOE::InputManager::GetMouseButtonUp(MouseButton button)
{
	return GetButtonUp(MouseButtonToKey(button));
}

/// <summary>
/// 키와 함수를 바인딩합니다
/// 
/// </summary>
/// <param name="ptr">함수를 제공하는 객체 포인터</param>
/// <param name="key">카</param>
/// <param name="state">상태</param>
/// <param name="fnc">함수</param>
void GOE::InputManager::BindAction(void* ptr, GOE::KEY key, KeyState state, std::function<void()> fnc)
{
	if (state == KeyState::DOWN) m_downListeners[key].emplace_back(ptr, fnc);
	else if (state == KeyState::UP) m_upListeners[key].emplace_back(ptr, fnc);
	// Press는 update내내 사용되므로 상태로서 각 객체가 관리하는게 효율적
}

/// <summary>
/// 바인딩된 함수를 해제합니다.
/// 특정키, 특정 상태
/// </summary>
/// <param name="ptr">함수를 제공한 객체</param>
/// <param name="key">키</param>
/// <param name="state">상태</param>
void GOE::InputManager::UnbindAction(void* ptr, KEY key, KeyState state)
{
	if (key < 0 || KeyState::NONE == state) return;
	if (m_isDispatching)
	{
		m_unbindingQueue.emplace_back([=]() { ReleaseAction(ptr, key, state); });
	}
	else
	{
		ReleaseAction(ptr, key, state);
	}
}

void GOE::InputManager::UnbindAction(void* ptr, KEY key)
{
	if (key < 0 ) return;
	if (m_isDispatching)
	{
		m_unbindingQueue.emplace_back([=]() { ReleaseAction(ptr, key, KeyState::NONE); });
	}
	else
	{
		ReleaseAction(ptr, key, KeyState::NONE);
	}
}

void GOE::InputManager::UnbindAction(void* ptr)
{
	if (m_isDispatching)
	{
		m_unbindingQueue.emplace_back([=]() { ReleaseAction(ptr, -1, KeyState::NONE); });
	}
	else
	{
		ReleaseAction(ptr, -1, KeyState::NONE);
	}
}

/// <summary>
/// 해당 Key가 UP상태일 경우
/// 바인딩된 함수들을 순서대로 실행합니다.
/// </summary>
/// <param name="key">키</param>
void GOE::InputManager::DispatchUpEvent(KEY key)
{
	// m_upListeners만 확인
	auto it = m_upListeners.find(key);
	if (it != m_upListeners.end())
	{
		m_isDispatching = true;
		for (const auto& func : it->second)
		{
			// 액션이 존재하면 실행한다.
			if (func.action)
			{
				func.action();
			}
		}
		m_isDispatching = false;
		FlushUnbindAcions();
	}
}

/// <summary>
/// 해당 Key가 DOWN상태일 경우
/// 바인딩된 함수들을 순서대로 실행합니다.
/// 
/// </summary>
/// <param name="key">키</param>
void GOE::InputManager::DispatchDownEvent(KEY key)
{
	// m_upListeners만 확인
	auto it = m_downListeners.find(key);
	if (it != m_downListeners.end())
	{
		m_isDispatching = true;
		for (const auto& func : it->second)
		{
			// 액션이 존재하면 실행한다.
			if (func.action)
			{
				func.action();
			}
		}
		m_isDispatching = false;
		FlushUnbindAcions();
	}
}

/// <summary>
/// 마우스버튼의 대한 
/// 가상키사용 편의를 위한 변환함수
/// </summary>
/// <param name="button"></param>
/// <returns></returns>
int GOE::InputManager::MouseButtonToKey(GOE::MouseButton button)
{
	switch (button)
	{
		case GOE::MouseButton::Left:   return VK_LBUTTON;
		case GOE::MouseButton::Right:  return VK_RBUTTON;
		case GOE::MouseButton::Middle: return VK_MBUTTON;
		default: return -1;
	}
}

/// <summary>
/// 함수를 해제합니다.
/// 함수를 등록한 객체(ptr)의 대하여
/// NONE은 모든상태
/// -1은 모든키
/// 의 대한 해제를 의미합니다.
///
/// </summary>
/// <param name="ptr">함수를 등록한 객체의 포인터</param>
/// <param name="key">키</param>
/// <param name="state">키 상태</param>
void GOE::InputManager::ReleaseAction(void* ptr, KEY key, KeyState state)
{
	// listener 벡터를 제공받아서 해당 포인터가 등록한 리스너를 삭제하는 함수
	auto removeListener = [ptr](std::vector<InputListener>& vec)
		{
			vec.erase(std::remove_if(vec.begin(), vec.end(),
				[ptr](const InputListener listener)
				{
					return listener.ptr == ptr;
				}), vec.end());
		};

	// down의 경우
	if (state == GOE::KeyState::DOWN || state == GOE::KeyState::NONE)
	{
		if (key == -1)
		{
			for (auto& pair : m_downListeners) removeListener(pair.second);
		}
		else
		{
			auto it = m_downListeners.find(key);
			if (it != m_downListeners.end()) removeListener(it->second);
		}
	}
	// up의 경우
	if (state == GOE::KeyState::UP || state == GOE::KeyState::NONE)
	{
		if (key == -1)
		{
			for (auto& pair : m_upListeners) removeListener(pair.second);
		}
		else
		{
			auto it = m_upListeners.find(key);
			if (it != m_upListeners.end()) removeListener(it->second);
		}
	}
}

/// <summary>
/// 저장된 해제함수를 일괄 실행합니다.
/// 
/// </summary>
void GOE::InputManager::FlushUnbindAcions()
{
	for (const auto& func : m_unbindingQueue)
	{
		func();
	}
	m_unbindingQueue.clear();
}
