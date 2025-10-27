#pragma once

struct VertexWeight
{
	VertexWeight() = default;
	VertexWeight(unsigned int id, float weight)
		: m_vertexId(id), m_weight(weight){}
	
	unsigned int m_vertexId;
	float m_weight;
};

	class Bone
{
public:
	Bone() = default;
	Bone(const std::string& name, size_t id, size_t meshid)
		: m_name(name), m_id(id), m_meshID(meshid), m_offsetTM(GOE::Matrix4x4::Identity()), m_rootNodeID(0), m_nodeID(0)
	{}
	~Bone();

public:
	inline const std::string& GetName() const { return m_name; }
	inline size_t GetID() const { return m_id; }
	inline GOE::Matrix4x4& GetBoneOffset() { return m_offsetTM; }
	inline size_t GetRootNode() { return m_rootNodeID; }
	inline size_t GetNode() { return m_nodeID; }
	inline unsigned int GetBoneIndex() { return m_boneIndex; }

public:
	inline void SetBoneOffset(GOE::Matrix4x4 transform) { m_offsetTM = transform; }
	inline void SetNode(size_t hash) { m_nodeID = hash; }
	inline void SetRootNode(size_t hash) { m_rootNodeID = hash; }
	inline void SetBoneIndex(unsigned int index) { m_boneIndex = index; }

private:
	const std::string m_name; // 노드 이름
	const size_t m_id; // 노드 ID
	const size_t m_meshID = 0; // 이 본이 속한 메쉬 ID (필요시 사용)
	unsigned int m_boneIndex = 0; // 본 인덱스 (쉐이더에서 사용할 용도)

	GOE::Matrix4x4 m_offsetTM;
	
	size_t m_rootNodeID;
	size_t m_nodeID;
	
};

