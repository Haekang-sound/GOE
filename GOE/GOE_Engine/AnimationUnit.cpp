#include "Engine_pch.h"
#include "AnimationUnit.h"

AnimationUnit::AnimationUnit(size_t ownerID, size_t id)
	:IComponent(ownerID, id)
{
}

AnimationUnit::~AnimationUnit() = default;
