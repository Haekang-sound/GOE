#pragma once
#include "SingletonBase.h"

namespace GOE
{
	/// <summary>
	/// 폴더를 순회하고 해쉬값을 반환하는 파일매니저
	/// </summary>
	class FileManager : public SingletonBase<FileManager>
	{
	public:
		FileManager();
		~FileManager();
		FileManager(const FileManager&) = delete;
		FileManager(FileManager&&) = delete;
		FileManager& operator=(const FileManager&) = delete;
		FileManager& operator=(FileManager&&) = delete;

	public:
		static std::vector<std::wstring> GetFileNamesInFolder(std::wstring folderPath);
		static std::vector<std::wstring> GetThisFileNamesInFolder(std::wstring folderPath, const std::wstring& extension);

		static size_t GetHash(const std::string& path);

	};
}



