#include "Camera.h"

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::OnUpdate(POINT center)
{
	// 마우스 x는 y축 회전
	// 마우스 y는 x축 회전
	POINT current;
	GetCursorPos(&current);
	SetCursorPos(center.x, center.y);

	if (m_rotation.x >= 360.f || m_rotation.x <= -360.f) m_rotation.x = 0.f;
	if (m_rotation.y >= 360.f || m_rotation.y <= -360.f) m_rotation.x = 0.f;

	m_rotation.x += (current.y - center.y) * m_roatateSpeed;
	m_rotation.y += (current.x - center.x) * m_roatateSpeed;

	XMVECTOR pos = XMLoadFloat3(&m_position);

	// right (local x축)
	XMVECTOR right = XMVector3Normalize(XMVectorSet(m_local._11, m_local._21, m_local._31, 0.0f));

	// up (local y축)
	XMVECTOR up = XMVector3Normalize(XMVectorSet(m_local._12, m_local._22, m_local._32, 0.0f));

	// front (local z축)
	XMVECTOR front = XMVector3Normalize(XMVectorSet(m_local._13, m_local._23, m_local._33, 0.0f));


	if (GetAsyncKeyState('W') & 0x8000)	pos += front * m_moveSpeed;
	if (GetAsyncKeyState('A') & 0x8000)	pos -= right * m_moveSpeed;
	if (GetAsyncKeyState('S') & 0x8000)	pos -= front * m_moveSpeed;
	if (GetAsyncKeyState('D') & 0x8000)	pos += right * m_moveSpeed;
	if (GetAsyncKeyState('Q') & 0x8000)	pos -= up * m_moveSpeed;
	if (GetAsyncKeyState('E') & 0x8000)	pos += up * m_moveSpeed;

	 XMStoreFloat3(&m_position,pos);

	// (1) 각 변환을 XMMATRIX로 생성
	XMMATRIX S = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	XMMATRIX R = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z); // 순서 유의
	XMMATRIX T = XMMatrixTranslationFromVector(pos);

	// (2) 합성
	XMMATRIX world = S * R * T;

	XMStoreFloat4x4(&m_local, XMMatrixInverse(nullptr, world));

}