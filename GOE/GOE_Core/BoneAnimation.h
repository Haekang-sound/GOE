#pragma once
class BoneAnimation
{
public:
	BoneAnimation() = default;
	BoneAnimation(std::string name, size_t hash) : m_name(name), m_id(hash) {}
	BoneAnimation(size_t hash) : m_id(hash) {}
	~BoneAnimation();

public:
	inline const std::string& GetName() const { return m_name; }
	inline const std::size_t GetID() const { return m_id; }
	inline const std::vector<GOE::FLoatVector3>& GetScales() { return m_scales; }
	inline const std::vector<GOE::FLoatVector3>& GetPositions() { return m_positions; }
	inline const std::vector<GOE::FLoatVector4>& GetRotations() { return m_rotations; }

public:
	inline void SetName(const std::string& name) { m_name = name; }

	inline void AddPositions(GOE::FLoatVector3 position) { m_positions.push_back(position);}
	inline void AddRotations(GOE::FLoatVector4 quat) { m_rotations.push_back(quat);}
	inline void AddScales(GOE::FLoatVector3 scale) { m_scales.push_back(scale);}

private:
	std::string m_name = "";
	size_t m_id = 0;

	std::vector<GOE::FLoatVector3> m_scales;
	std::vector<GOE::FLoatVector4> m_rotations;
	std::vector<GOE::FLoatVector3> m_positions;
};

