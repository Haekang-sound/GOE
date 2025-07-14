#pragma once
template<typename T>
class SingletonBase
{
public:
	static T& GetInstance()
	{
		static T instance;
		return instance;
	}

protected:
	SingletonBase() = default;
	~SingletonBase() = default;

	SingletonBase(const SingletonBase&) = delete;
	SingletonBase& operator=(const SingletonBase&) = delete;
};