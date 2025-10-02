#pragma once
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

public:
	inline void SetLocalTransfrom(GOE::Matrix4x4 transform) { m_transfrom = transform; }

private:
	const std::string m_name; // 노드 이름
	const size_t m_id; // 노드 ID

	GOE::Matrix4x4 m_transfrom;
};

