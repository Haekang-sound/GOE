#pragma once
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>

#include<iostream>
#include<filesystem>

class AssimpIOStream : public Assimp::IOStream
{
	friend class AssimpIOSystem;

protected:
	// Constructor protected for private usage by MyIOSystem
	AssimpIOStream() {};

public:
	~AssimpIOStream() {};
	size_t Read(void* pvBuffer, size_t pSize, size_t pCount);
	size_t Write(const void* pvBuffer, size_t pSize, size_t pCount);
	aiReturn Seek(size_t pOffset, aiOrigin pOrigin);
	size_t Tell() const;
	size_t FileSize() const;
	void Flush();
};

// Fisher Price - My First Filesystem
class AssimpIOSystem : public Assimp::IOSystem
{
public:
	AssimpIOSystem() {}
	~AssimpIOSystem() {}

	bool Exists(const char* pFile) const override
	{
		// Check if the file exists using std::filesystem
		return std::filesystem::exists(pFile);

	}

	virtual char getOsSeparator() const override
	{
		return '/'; // Assimp uses '/' as the path separator
	}

	Assimp::IOStream* Open(const char* pFile, const char* pMode = "rb")
	{
		return new AssimpIOStream();
	}

	void Close(Assimp::IOStream* pFile) override
	{
		delete pFile; 
	}
};

