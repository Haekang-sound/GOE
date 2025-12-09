#pragma once
#include "SingletonBase.h"
#include <functional> 
#include <map>
namespace GOE
{
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

	using EventID = size_t;
	// 액션을 정의합니다.
	struct InputListener
	{
		InputListener(EventID e_id, std::function<void()> act)
			:id(e_id), action(act)
		{
		}
		EventID id;
		std::function<void()> action;
	};

	/// <summary>
	/// 인풋을 관리합니다.
	/// Bind함수로 키에 기능을 바인딩하고 
	/// Update함수로 키의 상태를 검사하며 상태에 맞게 바인딩 된 함수를 실행합니다.
	/// 
	/// </summary>
	class InputManager : public SingletonBase<InputManager>
	{
	public:
		InputManager();
		~InputManager();

	public:
		void Initialize(HWND hwnd);
		void Update();

	public:
		// keybord
		bool GetButtonDown(int key);
		bool GetButton(int key);
		bool GetButtonUp(int key);
		// mouse
		bool GetMouseButtonDown(MouseButton button);
		bool GetMouseButton(MouseButton button);
		bool GetMouseButtonUp(MouseButton button);
		EventID BindAction(int key, KeyState state, std::function<void()> fnc);

		const POINT& GetMousePos() const { return m_mousePos; }
		const POINT& GetScreenMousePos() const { return m_screenMousePos; }
		const POINT& GetMouseDelta() const { return m_mouseDelta; }


	private:
		void DispatchUpEvent(int key);
		void DispatchDownEvent(int key);
		int MouseButtonToKey(MouseButton button);

	private:
		HWND m_hWnd;
		POINT m_screenMousePos = { 0, 0 };// 현재 좌표 (스크린 기준)
		POINT m_mousePos = { 0, 0 };      // 현재 좌표 (클라이언트 기준)
		POINT m_prevMousePos = { 0, 0 };  // 이전 프레임 좌표
		POINT m_mouseDelta = { 0, 0 };    // 이동량
		std::vector<KeyState> m_keyStates;
		std::map<int, std::vector<InputListener>> m_downListeners;
		std::map<int, std::vector<InputListener>> m_upListeners;
	};
}
