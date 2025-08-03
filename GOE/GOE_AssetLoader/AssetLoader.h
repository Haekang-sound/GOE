#pragma once
struct GOE::MeshData;

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
	bool LoadModelFromFile(const std::string& filePath, GOE::MeshData& outMeshData);

private: 
	void ProcessNode(aiNode* node, const aiScene* scene, GOE::MeshData& outMeshData);
	void ProcessMesh(aiMesh* mesh, const aiScene* scene, GOE::MeshData& outMeshData);
};