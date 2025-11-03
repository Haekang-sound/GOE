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

private:
	bool m_isAnimate = true; // 이동 가능 여부
	size_t animationHash = 0;
};


