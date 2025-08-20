#include "Engine_pch.h"
#include "MovementUnit.h"

MovementUnit::MovementUnit(size_t ownerID, size_t id)
	:IComponent(ownerID, id)
{
}

MovementUnit::~MovementUnit() = default;
