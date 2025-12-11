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

GOE::FLoatVector4 Transform::GetRightVector()
{
	return GOE::FLoatVector4(m_localTM._11, m_localTM._12, m_localTM._13, 0.0f);
}

GOE::FLoatVector4 Transform::GetUpVector()
{
	return GOE::FLoatVector4(m_localTM._21, m_localTM._22, m_localTM._23, 0.0f);
}

GOE::FLoatVector4 Transform::GetForwardVector()
{
	return GOE::FLoatVector4(m_localTM._31, m_localTM._32, m_localTM._33, 0.0f);
}