#pragma once
struct MeshData;

/// <summary>
/// 리소스를 로드하는 클래스
/// 
/// </summary>
class AssetLoader
{
public: 
	AssetLoader() {};
	~AssetLoader() {};

public: 
	bool LoadModelFromFile(const std::string& filePath, MeshData& outMeshData);
	void processMesh(aiMesh* mesh, const aiScene* scene, MeshData& outMeshData);
};

