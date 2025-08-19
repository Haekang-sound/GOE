#pragma once
#include <unordered_map>
#include <string>
#include <memory>

class Model;
class Node;
class Mesh;


struct aiNode;
struct aiMesh;
struct aiScene;

/// <summary>
/// 리소스를 로드하는 클래스
/// 
/// </summary>
class AssetLoader
{
public: 
	AssetLoader() {};
	~AssetLoader();

public: 
	bool LoadModelFromFile(const std::string& filePath);

public:
	const Model* GetModel(const std::string hash) 
	{	
		auto it = m_models.find(hasher(hash));
		if (m_models.find(hasher(hash)) != m_models.end())
        {
			return it->second.get(); // 찾았으면 모델의 raw pointer 반환
		}
		return nullptr; // 못 찾았으면 nullptr 반환
	}

	const std::unordered_map<std::size_t, std::unique_ptr<Model>>& GetModels() const 
	{
		return m_models; // 모델 맵 반환
	}

	const std::unordered_map<std::size_t, std::unique_ptr<Mesh>>& GetMeshes() const 
	{
		return m_meshes; // 메쉬 맵 반환
	}

	const Mesh* GetMesh(size_t hash) 
	{
		auto it = m_meshes.find(hash);
		if (it != m_meshes.end())
		{
			return it->second.get(); // 찾았으면 메쉬의 raw pointer 반환
		}
		return nullptr; // 못 찾았으면 nullptr 반환
	}
			
private: 
	std::unique_ptr<Node> ProcessNode(aiNode* node);
	std::unique_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);

private:
	std::unordered_map <std::size_t, std::unique_ptr<Model>> m_models;
	std::unordered_map <std::size_t, std::unique_ptr<Mesh>> m_meshes;

private:
	// std::string 타입을 해시할 수 있는 hasher 객체를 생성합니다.
	std::hash<std::string> hasher;
};