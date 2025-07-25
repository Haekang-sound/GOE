#pragma once
using namespace Assimp;

/// <summary>
/// 디버깅툴은 다다익선
/// 로그를 어떻게 기록할것인가? 
/// 
/// ohk 2025.07.24
/// </summary>
class AssimpLogger : public Assimp::LogStream
{
public:
	AssimpLogger();
	~AssimpLogger();

public:
	// Write something using your own functionality
	void write(const char* message)
	{
		::printf("%s\n", message);
	}
};

