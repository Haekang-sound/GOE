#pragma once
#include "SingletonBase.h"
#include <filesystem>
#include <string>
#include <string_view>

namespace GOE
{
	/// <summary>
	/// 폴더를 순회하고 해쉬값을 반환하는 파일매니저
	/// </summary>
	class FileManager : public SingletonBase<FileManager>
	{
	private:
		size_t m_IDValue = 0; 
		std::filesystem::path m_rootPath;   // 프로젝트 루트
		std::filesystem::path m_assetsPath; // Assets 폴더

	public: 
		void Initialize();


	public:
		std::string GetFullPath(const std::string_view path);
		std::wstring GetFullPath(const std::wstring_view path);

		static std::vector<std::wstring> GetFileNamesInFolder(std::wstring_view folderPath);
		static std::vector<std::wstring> GetThisFileNamesInFolder(std::wstring_view folderPath, const std::wstring& extension);

		static size_t GetHash(const std::string_view path);
		
		// id value가 필요할때사용함
		size_t GetRendomHash()
		{
			size_t hash = std::hash<size_t>{}(m_IDValue);
			m_IDValue++;
			return hash;
		};
	};
}



