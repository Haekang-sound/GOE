#pragma once
template<typename T>
class SingleTonBase
{
public:
	static T& GetInstance()
	{
		static T instance;
		return instance;
	}

protected:
	SingleTonBase() = default;
	~SingleTonBase() = default;

	SingletonBase(const SingletonBase&) = delete;
	SingletonBase& operator=(const SingletonBase&) = delete;
};