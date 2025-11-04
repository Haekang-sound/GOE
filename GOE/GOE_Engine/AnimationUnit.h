#pragma once
#include "IComponent.h"

class AnimationUnit : public IComponent
{
public:
	AnimationUnit(size_t ownerID, size_t id);
	~AnimationUnit();

public:
	inline const bool IsAnimated() const { return m_isAnimate; }
	inline const size_t GetAnimationHash() const { return animationHash; }
public:
	inline void SetAnimate(bool isMoveable) { m_isAnimate = isMoveable; }
	inline void SetAnimationHash(size_t hash) { animationHash = hash; }

protected:
	bool m_isAnimate = true; // 이동 가능 여부
	size_t animationHash = 0;

public:
	double m_duration = 0.0f;
	double m_ticksPerSecond = 0.0f;
	double m_totalTime = 0.0f;
	double m_totalTick = 0.0f;
	double m_normalizedTick = 0.0f;
	double m_previousNormalizedTick = 0.0f; // 루트 모션 계산에 필수
	
};


