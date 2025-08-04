#pragma once
#include <vector>
#include <string>
#include <memory>

#include "Mesh.h" 
#include "Node.h" 

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

	// 소멸자 (unique_ptr 때문에 가상 소멸자가 권장됩니다)
	virtual ~Model() = default;

	// ==========================================================
	//        여기에 "Rule of 5"를 적용합니다.
	// ==========================================================

	// 1. 복사 생성자 (삭제)
	Model(const Model&) = delete;

	// 2. 복사 대입 연산자 (삭제)
	Model& operator=(const Model&) = delete;

	// 3. 이동 생성자 (기본 구현 사용)
	//    std::move(otherModel) 호출 시 사용됩니다.
	Model(Model&&) noexcept = default;

	// 4. 이동 대입 연산자 (기본 구현 사용)
	//    myModel = std::move(otherModel) 호출 시 사용됩니다.
	Model& operator=(Model&&) noexcept = default;

public:
	inline void SetName(const std::string& name) { m_name = name; }
	inline const std::string& GetName() const { return m_name; }

	inline void SetID(std::size_t id) { id = id; }
	inline std::size_t GetID() const { return m_id; }

	inline void AddRootNode(Node&& node) { m_rootNode = std::move(node); }
	inline const Node& GetRootNode() const { return m_rootNode; }

	inline void AddMesh(Mesh mesh) { m_meshes.push_back(std::move(mesh)); }
	inline const std::vector<Mesh> GetMeshes() const { return m_meshes; }
	
private:
	std::string m_name; // 모델 이름
	size_t m_id;		// 모델 ID

	Node m_rootNode; 
	std::vector<Mesh> m_meshes;


};

