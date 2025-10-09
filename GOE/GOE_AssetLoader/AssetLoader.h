#pragma once
#include <unordered_map>
#include <string>
#include <memory>

class Model;
class Node;
class Mesh;
class Texture;
class Animation;
class Bone;

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;

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

	/// <summary>
	/// 애니메이션을 로드하는 함수
	/// </summary>
	/// <param name="filePath">파일경로</param>
	/// <returns></returns>
	bool LoadAnimiationFromFile(const std::string& filePath);
	
public:
	inline Model* GetModel(size_t hash)
	{
		auto it = m_models.find(hash);
		if (it != m_models.end())
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


	Animation* GetAnimation(size_t hash)
	{
		auto it = m_animations.find(hash);
		if (it != m_animations.end())
		{
			return it->second.get();
		}

		return nullptr;
	}

private:
	std::unique_ptr<Node> ProcessNode(aiNode* node, Node* parent = nullptr);
	std::unique_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);

private:
	std::unordered_map <std::size_t, std::unique_ptr<Model>> m_models;
	std::unordered_map <std::size_t, std::unique_ptr<Mesh>> m_meshes;
	std::unordered_map <std::size_t, std::unique_ptr<Animation>> m_animations;

private:
	GOE::Matrix4x4 aiMatrix4x4ToCoreMtrix(const aiMatrix4x4& nodeTM);
};