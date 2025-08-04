#pragma once

namespace GOE
{
	struct MeshData;
}

/// <summary>
/// 버텍스와 인덱스정보를 갖고 있는 매쉬
/// 
/// ohk 2025.07.29
/// </summary>
class Mesh
{
	public:
	Mesh() = default;
	Mesh(const std::string& name, size_t id)
		: m_name(name), m_id(id){}

	~Mesh() = default;

public:
	inline void SetName(const std::string& name) { m_name = name; }
	inline const std::string& GetName() const { return m_name; }

	inline void SetID(size_t id) { m_id = id; }
	inline const size_t GetID() const { return m_id; }

	inline void SetMeshData(GOE::MeshData&& meshData) { m_meshData = std::move(meshData); }
	inline const GOE::MeshData GetMeshData() const { return m_meshData; }

private:
	std::string m_name; // 매쉬 이름
	size_t m_id; // 매쉬 ID
	GOE::MeshData m_meshData;// = nullptr; // 매쉬 데이터
};

