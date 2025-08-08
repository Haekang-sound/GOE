#pragma once
#include <vector>
#include <string>
#include <memory>

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
	inline void SetName(const std::string& name) { m_name = name; }
	inline const std::string& GetName() const { return m_name; }

	inline std::size_t GetID() const { return m_id; }

	inline void AddRootNode(std::unique_ptr<Node>&& node) { m_rootNode = std::move(node); }
	inline const std::unique_ptr<Node>& GetRootNode() const { return m_rootNode; }

	inline void AddMesh(std::unique_ptr<Mesh>&& mesh) { m_meshes.push_back(std::move(mesh)); }
	inline const std::vector<std::unique_ptr<Mesh>>& GetMeshes() const { return m_meshes; }
	
private:
	std::string m_name = ""; // 모델 이름
	const size_t m_id;		// 모델 ID

	std::unique_ptr<Node> m_rootNode; 
	std::vector<std::unique_ptr<Mesh>> m_meshes;
};

