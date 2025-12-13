#pragma once
#include "IComponent.h"
#include "BoneAnimation.h"


class AnimationUnit : public IComponent
{
protected:
	bool m_isAnimate = true;
	size_t animationHash = 0;

public:
	double m_duration = 0.0f;
	double m_ticksPerSecond = 0.0f;
	double m_totalTime = 0.0f;
	double m_totalTick = 0.0f;
	double m_normalizedTick = 0.0f;
	double m_previousNormalizedTick = 0.0f; // 루트 모션 계산에 필수
	// 각 본의 마지막 인덱스를 저장하는벡터
	// 보간함수는 저장된 인덱스를 참조하고 
	// 여기부터 검사하면서 효율적으로 적절한 프레임을 찾습니다.
	std::vector<BoneCache> m_cachedBoneIndices;

public:
	AnimationUnit(size_t ownerID, size_t id);
	~AnimationUnit();

public:
	inline const bool IsAnimated() const { return m_isAnimate; }
	inline const size_t GetAnimationHash() const { return animationHash; }
public:
	inline void SetAnimate(bool isMoveable) { m_isAnimate = isMoveable; }
	inline void SetAnimationHash(size_t hash) { animationHash = hash; }
};


