#pragma once
#include "SingletonBase.h"

namespace GOE
{
	class TimeManager : public SingletonBase<TimeManager>
	{
	public:
		TimeManager();
		~TimeManager();

	public:
		void Initialize();
		void Update();

	public:
		inline double GetDeltaTime() { return m_deltaTime; }
		inline double GetFPS() { return 1.0 / m_deltaTime; }

	private:
		std::chrono::high_resolution_clock::time_point m_prevTime;
		double m_deltaTime;
	};
}