#include "Engine_pch.h"
#include "CameraComponent.h"
#include "Transform.h"

CameraComponent::CameraComponent(size_t ownerID, size_t id)
	: IComponent(ownerID, id)
{
	// 기본값 설정
	// GOE::Matrix4x4::PerspectiveFovLH는 보통 Radian을 받음.
	m_fov = 0.785398f; // 45도 (Radian)
}

CameraComponent::~CameraComponent()
{
}
