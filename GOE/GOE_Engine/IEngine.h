#pragma once
#include<Windows.h>
struct UILoopInfo;
struct UIInitInfo;
class SceneManager;
namespace GOE
{
	class IEngine
	{
	public:
		IEngine() = default;
		virtual ~IEngine() = default;

	public:
		virtual void Initialize() = 0;

		virtual void OnUpdate() = 0;

		virtual void BeginRender() = 0;
		virtual void OnRender() = 0;
		virtual void EndRender() = 0;

		virtual void Release() = 0;

	public:
		virtual SceneManager* GetSceneManager() = 0;
		virtual UIInitInfo* GetUIInfo() = 0;
		virtual UILoopInfo* GetUILoopInfo() = 0;
		virtual HWND GetHWND() = 0;
	};

};