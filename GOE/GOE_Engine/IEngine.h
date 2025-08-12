#pragma once

namespace GOE
{
	class IEngine
	{
	public:
		IEngine() = default;
		virtual ~IEngine() = default;

	public:
		virtual void Initialize() = 0;
		
		virtual void OnUpdate(double dTime) = 0;

		virtual void BeginRender() = 0;
		virtual void OnRender() = 0;
		virtual void EndRender() = 0;

	};
}