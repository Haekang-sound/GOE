#pragma once
#include "SingletonBase.h"
#include <functional> 
#include <map>

namespace GOE
{
	using KEY = int;

	enum class MouseButton
	{
		Left = 0,
		Right,
		Middle
	};

	enum class KeyState
	{
		NONE = 0,
		DOWN,
		PRESSED,
		UP
	};

	// 액션을 정의합니다.
	struct InputListener
	{
		InputListener(void* _ptr, std::function<void()> act)
			:ptr(_ptr), action(act){}
		void* ptr;
		std::function<void()> action;
	};

	/// <summary>
	/// 인풋을 관리합니다.
	/// Bind함수로 키에 기능을 바인딩하고 
	/// Update함수로 키의 상태를 검사하며
	/// 상태에 맞게 바인딩 된 함수를 실행합니다.
	/// 
	/// </summary>
	class InputManager : public SingletonBase<InputManager>
	{
	public:
		InputManager();
		~InputManager();

	private:
		HWND m_hWnd;
		POINT m_screenMousePos = { 0, 0 };// 현재 좌표 (스크린 기준)
		POINT m_mousePos = { 0, 0 };      // 현재 좌표 (클라이언트 기준)
		POINT m_prevMousePos = { 0, 0 };  // 이전 프레임 좌표
		POINT m_mouseDelta = { 0, 0 };    // 이동량
		std::vector<KeyState> m_keyStates;
		std::map<KEY, std::vector<InputListener>> m_downListeners;
		std::map<KEY, std::vector<InputListener>> m_upListeners;
		
		bool m_isDispatching = false;
		// 키할당 제거함수를 담아두는 큐
		std::vector<std::function<void()>> m_unbindingQueue;

	public:
		void Initialize(HWND hwnd);
		void Update();

	public:
		// keybord
		bool GetButtonDown(KEY key);
		bool GetButton(KEY key);
		bool GetButtonUp(KEY key);
		// mouse
		bool GetMouseButtonDown(MouseButton button);
		bool GetMouseButton(MouseButton button);
		bool GetMouseButtonUp(MouseButton button);

		const POINT& GetMousePos() const { return m_mousePos; }
		const POINT& GetScreenMousePos() const { return m_screenMousePos; }
		const POINT& GetMouseDelta() const { return m_mouseDelta; }

	// 함수할당, 해제
	public:
		void BindAction(void* ptr, KEY key, KeyState state, std::function<void()> fnc);
		// 해제큐에 특정키의 대한 해제함수를 담아두는 역할을 한다.
		void UnbindAction(void* ptr, KEY key, KeyState state);
		void UnbindAction(void* ptr, KEY key);
		void UnbindAction(void* ptr);

	private:
		void DispatchUpEvent(KEY key);
		void DispatchDownEvent(KEY key);
		int MouseButtonToKey(MouseButton button);

		void ReleaseAction(void* ptr, KEY key, KeyState state);
		void FlushUnbindAcions();
	};
}
