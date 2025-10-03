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
	Bone(const std::string& name, size_t id)
		: m_name(name), m_id(id)
	{}
	~Bone();

public:
	inline const std::string& GetName() const { return m_name; }
	inline size_t GetID() const { return m_id; }
	inline GOE::Matrix4x4& GetBoneOffset() { return m_offsetTM; }
	inline std::vector<VertexWeight>& GetWeights() { return m_weights; }
	inline size_t GetRootNode() { return m_rootNode; }
	inline size_t GetNode() { return m_node; }

public:
	inline void SetBoneOffset(GOE::Matrix4x4 transform) { m_offsetTM = transform; }
	inline void AddWeight(float vertexId, float weight) { m_weights.emplace_back(vertexId, weight); }
	inline void SetNode(size_t hash) { m_node = hash; }
	inline void SetRootNode(size_t hash) { m_rootNode = hash; }

private:
	const std::string m_name; // 노드 이름
	const size_t m_id; // 노드 ID

	GOE::Matrix4x4 m_offsetTM;
	
	size_t m_rootNode;
	size_t m_node;
	
	std::vector<VertexWeight> m_weights;

};

