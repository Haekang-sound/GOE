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
	inline GOE::Matrix4x4 GetSRTMatrix(size_t index)
	{
		// 1. 각 변환을 위한 행렬을 단위 행렬로 초기화합니다.
		GOE::Matrix4x4 scaleMatrix = GOE::Matrix4x4::Identity();
		GOE::Matrix4x4 rotationMatrix = GOE::Matrix4x4::Identity();
		GOE::Matrix4x4 translationMatrix = GOE::Matrix4x4::Identity();

		// 2. 스케일 벡터가 비어있지 않은 경우에만 변환 행렬을 계산합니다.
		if (!m_scales.empty())
		{
			scaleMatrix = m_scales[index % m_scales.size()].ToScaleMatrix();
		}

		// 3. 회전 벡터가 비어있지 않은 경우에만 변환 행렬을 계산합니다.
		if (!m_rotations.empty())
		{
			rotationMatrix = m_rotations[index % m_rotations.size()].ToRotationXMatrix();
		}

		// 4. 위치 벡터가 비어있지 않은 경우에만 변환 행렬을 계산합니다.
		if (!m_positions.empty())
		{
			translationMatrix = m_positions[index % m_positions.size()].ToTranslationMatrix();
		}

		// 5. 최종 변환 행렬을 조합하여 반환합니다.
		GOE::Matrix4x4 temp = scaleMatrix * rotationMatrix * translationMatrix;
		
		return temp;
	}


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

