#include "Engine_pch.h"
#include "Entity.h"

Entity::Entity(EntityID id, std::string name)
	: m_entityID(id), m_name(name)
{
}

Entity::~Entity() = default;