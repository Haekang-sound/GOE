#pragma once

namespace GOE
{
	struct MeshData;
}
class Bone;

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
	inline size_t GetModelID() const { return m_modelID; }
	inline size_t GetMeshIndex() const { return m_meshIndex; }
	const GOE::MeshData& GetMeshData() const { return *m_meshData.get(); };
	inline const std::vector<std::unique_ptr<Bone>>& GetBones() const { return m_bones; }
	inline const Bone* GetBoneByID(size_t boneID) const 
	{ 
		auto it = m_boneMap.find(boneID);
		if (it != m_boneMap.end())
		{
			return it->second; // 찾았으면 본의 raw pointer 반환
		}
		return nullptr; // 못 찾았으면 nullptr 반환
	}
public: 
	inline void SetModelID(size_t modelID) { m_modelID = modelID; }
	inline void SetMeshIndex(size_t meshIndex) { m_meshIndex = meshIndex; }
	inline void SetMeshData(std::unique_ptr<GOE::MeshData>&& meshData) { m_meshData = std::move(meshData); }
	inline void AddBone(std::unique_ptr<Bone>&& bone) { m_bones.push_back(std::move(bone)); }
	inline void AddBoneToMap(size_t boneID, Bone* bone) { m_boneMap[boneID] = bone; }
private:
	const std::string m_name; // 매쉬 이름
	const size_t m_id; // 매쉬 ID
	size_t m_modelID = 0; // 모델 ID (추가 필요시 사용)
	size_t m_meshIndex = 0; // 매쉬 인덱스 (추가 필요시 사용)
	
	std::vector<std::unique_ptr<Bone>> m_bones;
	std::unordered_map<size_t, Bone*> m_boneMap; // 본 해쉬맵
	std::unique_ptr<GOE::MeshData> m_meshData; // 매쉬 데이터 포인터
};

