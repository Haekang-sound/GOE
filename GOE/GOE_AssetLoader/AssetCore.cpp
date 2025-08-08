#include "AssetLoader_pch.h"
#include "AssetCore.h"
#include "AssetLoader.h"

//#include <assimp/Importer.hpp> // assimp 헤더는 cpp 파일에서만 포함!


AssetCore::AssetCore(){}
AssetCore::~AssetCore() = default; // 소멸자 구현

void AssetCore::CreateAssetLoader()
{
	m_assetLoader = std::make_unique<AssetLoader>();
}
void AssetCore::LoadModel(const std::string& filePath)
{
	m_assetLoader.get()->LoadModelFromFile(filePath);
}

const std::unordered_map<std::size_t, std::unique_ptr<Model>>& AssetCore::GetModels() const
{
	return m_assetLoader.get()->GetModels();
}
