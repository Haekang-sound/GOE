#include "Core_pch.h"
#include "FileManager.h"

GOE::FileManager::FileManager() = default;
GOE::FileManager::~FileManager() = default;


/// <summary>
/// 경로를 입력받고
/// 해당 폴더의 모든파일의 이름을 가져옵니다.
/// </summary>
/// <param name="folderPath">폴더 경로</param>
/// <returns></returns>
std::vector<std::wstring> GOE::FileManager::GetFileNamesInFolder(std::wstring folderPath)
{
	// 벡터의 타입도 std::wstring으로 변경합니다.
	std::vector<std::wstring> fileNames;

	// 지정된 경로에 있는 모든 파일/폴더를 순회합니다.
	for (const auto& entry : std::filesystem::directory_iterator(folderPath))
	{
		// entry가 디렉터리가 아닌 일반 파일일 경우에만 처리합니다.
		if (entry.is_regular_file())
		{
			// wstring 객체를 벡터에 push_back 합니다.
			// 여기서 생성된 임시 wstring 객체의 내용이 벡터 내부의 새로운 wstring 객체로 '이동(move)'되어 효율적입니다.
			fileNames.push_back(entry.path().filename().wstring());
		}
	}

	return fileNames;
}

/// <summary>
/// 경로와 확장자를 입력받고 해당폴더에서
/// 특정확장자를 가진 파일의 이름을 가져옵니다.
/// </summary>
/// <param name="folderPath">폴더경로</param>
/// <param name="extension">확장자</param>
/// <returns></returns>
std::vector<std::wstring> GOE::FileManager::GetThisFileNamesInFolder(std::wstring folderPath, const std::wstring& extension)
{
	std::vector<std::wstring> fileNames;

	try
	{
		for (const auto& entry : std::filesystem::directory_iterator(folderPath))
		{
			// 일반 파일이면서, 확장자가 일치하는 경우에만 처리합니다.
			if (entry.is_regular_file() && entry.path().extension() == extension)
			{
				fileNames.push_back(entry.path().filename().wstring());
			}
		}
	}
	catch (const std::filesystem::filesystem_error& e)
	{
		std::wcerr << L"FileManager Error: " << e.what() << std::endl;
	}

	return fileNames;
}

/// <summary>
/// 문자열을 입력받고 해쉬로 만들어줍니다.
/// </summary>
/// <param name="path"></param>
/// <returns></returns>
size_t GOE::FileManager::GetHash(const std::string& path)
{
	return std::hash<std::string>{}(path);
}