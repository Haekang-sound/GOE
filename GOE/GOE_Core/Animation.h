#pragma once
#include<memory>

class BoneAnimation;

/// 애니메이션 정보를 담을 클래스 애니메이션
/// 
class Animation
{
public: 
	Animation() = default;
	Animation(std::string name, size_t hash) : m_name(name), m_id(hash) {}
	Animation(size_t hash) : m_id(hash) {}
	~Animation();

public:
	inline const std::string& GetName() const { return m_name; }
	inline const std::size_t GetID() const { return m_id; }
	inline const double GetDuration() const { return m_Duration; }
	inline const double GetTicksPerSecond() const { return m_TicksPerSecond; }
	inline std::vector<std::unique_ptr<BoneAnimation>>& GetBoneAnimation() { return m_BoneAnimations; }

public:
	inline void SetName(const std::string& name) { m_name = name; }
	inline void SetDuation(const double value) { m_Duration = value; }
	inline void SetTicksPerSecond(const double value) { m_TicksPerSecond = value; }

	inline void AddBoneAnimation(std::unique_ptr<BoneAnimation>&& anim) { m_BoneAnimations.push_back(std::move(anim));}

private:
	std::string m_name = "";
	size_t m_id = 0;

	double m_Duration = 0.0;
	double m_TicksPerSecond = 0.0;

	// 본트랜스폼을 담는 boneanimation
	std::vector<std::unique_ptr<BoneAnimation>> m_BoneAnimations;

};

