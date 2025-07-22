#pragma once
#include "../GOE_Core/Commons.h"
#include <string>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class AssetLoader
{
public: 
	AssetLoader() {};
	~AssetLoader() {};

public: 
	bool LoadModelFromFile(const std::string& filePath, MeshData& outMeshData);
	void ProcessNode(aiNode* node, const aiScene* scene, const aiMatrix4x4& parentTransform, MeshData& outMeshData);
	void ProcessMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4& transform, MeshData& outMeshData);
};

