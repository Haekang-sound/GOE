#pragma once
#include "IComponent.h"
#include "Commons.h"

class Transform;

class CameraComponent : public IComponent
{
public:
	CameraComponent(size_t ownerID, size_t id);
	~CameraComponent();

public:
	void SetFOV(float fov) { m_fov = fov; }
	void SetAspectRatio(float aspect) { m_aspectRatio = aspect; }
	void SetNearZ(float nearZ) { m_nearZ = nearZ; }
	void SetFarZ(float farZ) { m_farZ = farZ; }

	float GetFOV() const { return m_fov; }
	float GetAspectRatio() const { return m_aspectRatio; }
	float GetNearZ() const { return m_nearZ; }
	float GetFarZ() const { return m_farZ; }

private:
	float m_fov = 45.0f; // Degree
	float m_aspectRatio = 1.6f; // 1280/800
	float m_nearZ = 0.1f;
	float m_farZ = 1000.0f;
};
