#include "Core_pch.h"
#include "FileManager.h"
namespace fs = std::filesystem; // 편의를 위한 별칭

void GOE::FileManager::Initialize()
{
	// 실행파일 경로를 가져옵니다.
	fs::path currentPath = fs::current_path();

	// 루트경로 찾기
	// asset폴더를 찾을때까지 상위폴더로 올라갑니다.
	fs::path searchPath = currentPath;
	bool found = false;
	// 무한루프 방지를 위해 5단계까지만 
	for (int i = 0; i < 5; ++i)
	{
		if (fs::exists(searchPath / "Assets"))
		{
			m_rootPath = searchPath;
			found = true;
			break;
		}
		if (searchPath.has_parent_path())
		{
			searchPath = searchPath.parent_path();
		}
	}

	// 찾지 못했다면
	if (!found)
	{
		m_rootPath = currentPath;
		std::cout << "[FileManager] Warning: Could not find 'Assets' folder. Root set to: " << m_rootPath << std::endl;
	}
	else
	{
		std::cout << "[FileManager] Root Path Initialized: " << m_rootPath << std::endl;
	}

}

std::string GOE::FileManager::GetFullPath(const std::string_view path)
{
	fs::path _path(path);
	if (_path.is_absolute()) return _path.string();

	return (m_rootPath/_path).string();
}

std::wstring GOE::FileManager::GetFullPath(const std::wstring_view path)
{
	fs::path _path(path);
	if (_path.is_absolute()) return _path.wstring();

	return (m_rootPath / _path).wstring();
}

/// <summary>
/// 경로를 입력받고
/// 해당 폴더의 모든파일의 이름을 가져옵니다.
/// </summary>
/// <param name="folderPath">폴더 경로</param>
/// <returns></returns>
std::vector<std::wstring> GOE::FileManager::GetFileNamesInFolder(std::wstring_view folderPath)
{
	// 벡터의 타입도 std::wstring으로 변경합니다.
	std::vector<std::wstring> fileNames;
	fs::path path(folderPath);

	if (!fs::exists(path)) return fileNames;

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
std::vector<std::wstring> GOE::FileManager::GetThisFileNamesInFolder(std::wstring_view folderPath, const std::wstring& extension)
{
	std::vector<std::wstring> fileNames;
	fs::path path(folderPath);

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
size_t GOE::FileManager::GetHash(const std::string_view path)
{
	return std::hash<std::string_view>{}(path);
}
