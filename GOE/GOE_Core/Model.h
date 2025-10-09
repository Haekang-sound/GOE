#pragma once
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
using MESH_ID = std::size_t;
using MESH_INDEX = std::size_t;

class Node;
class Mesh;
class Bone;

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
	Model(size_t hash) : m_id(hash) {}

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;
	Model(Model&&) noexcept = default;
	Model& operator=(Model&&) noexcept = default;

	~Model();

public:	
	inline const std::string& GetName() const { return m_name; }
	inline const std::size_t GetID() const { return m_id; }
	inline const std::unique_ptr<Node>& GetRootNode() const { return m_rootNode; }
	inline std::vector<Node*>& GetNodeVector() { return m_nodes; }
	inline Node* GetNodeFromMap(size_t id)
	{
		auto it = m_nodeMap.find(id);
		if (it != m_nodeMap.end())
		{
			return it->second; // 찾았으면 노드의 raw pointer 반환
		}
		return nullptr; // 못 찾았으면 nullptr 반환
	}
	inline const std::vector<MESH_ID>& GetMeshIDs() const { return m_meshIDs; }


public:
	inline void SetName(const std::string& name) { m_name = name; }
	inline void AddRootNode(std::unique_ptr<Node>&& node) { m_rootNode = std::move(node); }
	inline void AddNodeToVector(Node* node) { m_nodes.push_back(node); }
	inline void AddNodeToMap(size_t id, Node* node) { m_nodeMap[id] = node; }

	inline void AddMeshID(MESH_ID id) { m_meshIDs.push_back(id); }
	inline void AddMeshToMap(MESH_ID id, MESH_INDEX index)
	{
		m_meshMap[id] = index; // 매쉬 ID와 인덱스를 해시맵에 추가
	}
	
public: 
	void UpdateNodeHierarchy() { UpdateHierarchy(m_rootNode.get()); }

private:
	void UpdateHierarchy(Node* node);

private:
	std::string m_name = ""; // 모델 이름
	const size_t m_id;		// 모델 ID

	std::unique_ptr<Node> m_rootNode; 
	std::vector<Node*> m_nodes;
	std::unordered_map<size_t, Node*> m_nodeMap;

	std::vector<MESH_ID> m_meshIDs;
	std::unordered_map<MESH_ID, MESH_INDEX> m_meshMap; // 매쉬 이름과 인덱스를 매핑하는 해시맵
};

