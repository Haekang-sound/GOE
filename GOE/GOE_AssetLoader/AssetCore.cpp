#include "AssetCore.h"
#include "AssetLoader_pch.h"
#include <assimp/Importer.hpp> // assimp 헤더는 cpp 파일에서만 포함!
#include "AssetLoader.h"
#include "../GOE_Core/CoreMath.h"

AssetCore::AssetCore(){}
AssetCore::~AssetCore() = default; // 소멸자 구현

void AssetCore::CreateAssetLoader()
{
	m_assetLoader = std::make_unique<AssetLoader>();
}
void AssetCore::LoadModel(const std::string& filePath, GOE::MeshData& outMeshData)
{
	m_assetLoader.get()->LoadModelFromFile(filePath, outMeshData);
}
