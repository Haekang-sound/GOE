#pragma once
#include <unordered_map>
#include <string>
#include <memory>

#include <../GOE_Core/Model.h>

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
	bool LoadModelFromFile(const std::string& filePath);

private: 
	Node ProcessNode(aiNode* node);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

private:
	std::unordered_map<std::size_t, Model> m_models;

private:
	// 1. std::string 타입을 해시할 수 있는 hasher 객체를 생성합니다.
	std::hash<std::string> hasher;
};