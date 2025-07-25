#include "AssetLoader_pch.h"
#include "AssimpIOStream.h"


size_t AssimpIOStream::Read(void* pvBuffer, size_t pSize, size_t pCount)
{
	return 0; // Placeholder implementation
}
size_t AssimpIOStream::Write(const void* pvBuffer, size_t pSize, size_t pCount)
{
	return 0; // Placeholder implementation
}
aiReturn AssimpIOStream::Seek(size_t pOffset, aiOrigin pOrigin)
{
	return aiReturn_SUCCESS; // Placeholder implementation
}
size_t AssimpIOStream::Tell() const
{
	return 0; // Placeholder implementation
}
size_t AssimpIOStream::FileSize() const
{
	return 0; // Placeholder implementation
}
void AssimpIOStream::Flush()
{
	
}

