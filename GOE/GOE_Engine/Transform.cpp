#include "Engine_pch.h"
#include "Transform.h"

Transform::Transform(size_t ownerID, size_t id)
	:IComponent(ownerID, id),
	m_localTM{}, m_parentTM{},
	m_scale{1,1,1}, m_rotation{0,0,0}, m_position{0,0,0}
{
	m_localTM._11 = 1;
	m_localTM._22 = 1;
	m_localTM._33 = 1;
	m_localTM._44 = 1;
}
Transform::~Transform() = default;