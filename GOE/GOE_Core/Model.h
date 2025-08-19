#pragma once
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
using MESH_ID = std::size_t;
using MESH_INDEX = std::size_t;

class Node;
class Mesh;

/// <summary>
/// 모델링에 필요한 
/// 정보를 갖고 있는 클래스
/// 
/// ohk 2025.07.29
/// </summary>
class Model
{
public:
	// 생성자
	Model() = default;
	explicit Model(size_t hash) : m_id(hash) {}

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	Model(Model&&) noexcept = default;
	Model& operator=(Model&&) noexcept = default;

	~Model();

public:	
	inline const std::string& GetName() const { return m_name; }
	inline std::size_t GetID() const { return m_id; }

	inline const std::unique_ptr<Node>& GetRootNode() const { return m_rootNode; }

	//inline const std::vector<std::unique_ptr<Mesh>>& GetMeshes() const { return m_meshes; }
	//inline const std::unordered_map<MESH_ID, MESH_INDEX>& GetMeshMap() const { return m_meshMap; }
	//inline Mesh* GetMeshByID(MESH_ID id) 
	//{
	//	auto it = m_meshMap.find(id);
	//	if (it != m_meshMap.end())
	//	{
	//		return m_meshes[it->second].get(); // 해당 ID의 매쉬를 반환
	//	}
	//	return nullptr; // 찾지 못했을 경우 nullptr 반환
	//}

public:
	inline void SetName(const std::string& name) { m_name = name; }
	inline void AddRootNode(std::unique_ptr<Node>&& node) { m_rootNode = std::move(node); }
	//inline void AddMesh(std::unique_ptr<Mesh>&& mesh) { m_meshes.push_back(std::move(mesh)); }
	//inline void AddMeshToMap(MESH_ID id, MESH_INDEX index) 
	//{ 
	//	m_meshMap[id] = index; // 매쉬 ID와 인덱스를 해시맵에 추가
	//}
	
private:
	std::string m_name = ""; // 모델 이름
	const size_t m_id;		// 모델 ID

	std::unique_ptr<Node> m_rootNode; 
	//std::vector<MESH_ID> m_meshIDs;
	//std::vector<std::unique_ptr<Mesh>> m_meshes;
	//std::unordered_map<MESH_ID, MESH_INDEX> m_meshMap; // 매쉬 이름과 인덱스를 매핑하는 해시맵
};

