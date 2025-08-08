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
	Mesh(const std::string name, size_t id)
		: m_name(name), m_id(id){}

	~Mesh();

public:
	inline const std::string& GetName() const { return m_name; }
	inline const size_t GetID() const { return m_id; }

	inline void SetMeshData(std::unique_ptr<GOE::MeshData>&& meshData) { m_meshData = std::move(meshData); }
	const GOE::MeshData& GetMeshData();// { return *m_meshData.get(); }

private:
	const std::string m_name; // 매쉬 이름
	const size_t m_id; // 매쉬 ID

	std::unique_ptr<GOE::MeshData> m_meshData; // 매쉬 데이터 포인터
};

