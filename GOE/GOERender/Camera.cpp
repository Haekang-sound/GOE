#include "Camera.h"
#include <iostream>
Camera::Camera()
{
	XMStoreFloat4x4(&m_local, XMMatrixIdentity());
}

Camera::~Camera()
{
}

void Camera::OnUpdate(POINT center)
{
	// 기저벡터
	XMVECTOR right = XMVector3Normalize(XMVectorSet(m_local._11, m_local._21, m_local._31, 0.0f));
	XMVECTOR up = XMVector3Normalize(XMVectorSet(m_local._12, m_local._22, m_local._32, 0.0f));
	XMVECTOR forward = XMVector3Normalize(XMVectorSet(m_local._13, m_local._23, m_local._33, 0.0f));

	// 마우스 x는 y축 회전
	// 마우스 y는 x축 회전
	POINT current;
	GetCursorPos(&current);
	SetCursorPos(center.x, center.y);

	XMMATRIX R = {};
	if (true)
	{
		m_rotation.x += (current.y - center.y) * m_roatateSpeed;
		m_rotation.y += (current.x - center.x) * m_roatateSpeed;
		R = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
		
	}
	else
	{
		// 카메라 회전
		XMVECTOR rotQX = XMQuaternionRotationAxis(right, (current.y - center.y) * m_roatateSpeed);
		XMVECTOR rotQY = XMQuaternionRotationAxis(up, (current.x - center.x) * m_roatateSpeed);
		m_quat = XMQuaternionNormalize(XMQuaternionMultiply(m_quat, rotQX));
		m_quat = XMQuaternionNormalize(XMQuaternionMultiply(m_quat, rotQY));
		R = XMMatrixRotationQuaternion(m_quat);
	}
	
	// 카메라 이동
	XMVECTOR pos = XMLoadFloat3(&m_position);
	if (GetAsyncKeyState('W') & 0x8000)	pos += forward * m_moveSpeed;
	if (GetAsyncKeyState('A') & 0x8000)	pos -= right * m_moveSpeed;
	if (GetAsyncKeyState('S') & 0x8000)	pos -= forward * m_moveSpeed;
	if (GetAsyncKeyState('D') & 0x8000)	pos += right * m_moveSpeed;
	if (GetAsyncKeyState('Q') & 0x8000)	pos -= up * m_moveSpeed;
	if (GetAsyncKeyState('E') & 0x8000)	pos += up * m_moveSpeed;

	// (1) 각 변환을 XMMATRIX로 생성
	XMMATRIX S = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	
	XMMATRIX T = XMMatrixTranslationFromVector(pos);
	
	// (2) 합성
	XMMATRIX world = S * R * T;

	XMStoreFloat4x4(&m_local, XMMatrixInverse(nullptr, world));
	XMStoreFloat3(&m_position,pos);
}