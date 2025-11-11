#include "Renderer_pch.h"
#include "Camera.h"

#include "../GOE_Editor/DebugManager.h"
#include "../Imgui/imgui.h"

Camera::Camera(HWND hWnd)
	: m_hWnd(hWnd)
{
	XMStoreFloat4x4(&m_local, XMMatrixIdentity());
}

Camera::Camera()
{
	XMStoreFloat4x4(&m_local, XMMatrixIdentity());
}

Camera::~Camera()
{
}

void Camera::OnUpdate()
{
	// 기저벡터
	XMVECTOR right = XMVector3Normalize(XMVectorSet(m_local._11, m_local._21, m_local._31, 0.0f));
	XMVECTOR up = XMVector3Normalize(XMVectorSet(m_local._12, m_local._22, m_local._32, 0.0f));
	XMVECTOR forward = XMVector3Normalize(XMVectorSet(m_local._13, m_local._23, m_local._33, 0.0f));

	if(GetAsyncKeyState(VK_RBUTTON) & 0x8000)
	{
		GetCursorPos(&m_currentMouse);

		if (!m_keydown)
		{
			m_prevMouse = m_currentMouse;
			m_keydown = true;
		}
	}
	else
	{
		m_keydown = false;
	}

	RECT rect;
	GetClientRect(m_hWnd, &rect);

	int centerX = (rect.right - rect.left) / 2;
	int centerY = (rect.bottom - rect.top) / 2;

	POINT center = { centerX, centerY };
	ClientToScreen(m_hWnd, &center);

	XMMATRIX R = {};
	if (true)
	{
		m_rotation.x += (m_currentMouse.y - m_prevMouse.y) * m_roatateSpeed;
		m_rotation.y += (m_currentMouse.x - m_prevMouse.x) * m_roatateSpeed;
		R = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
		
	}
	else
	{
		// 카메라 회전
		XMVECTOR rotQX = XMQuaternionRotationAxis(right, (m_currentMouse.y - center.y) * m_roatateSpeed);
		XMVECTOR rotQY = XMQuaternionRotationAxis(up, (m_currentMouse.x - center.x) * m_roatateSpeed);
		m_quat = XMQuaternionNormalize(XMQuaternionMultiply(m_quat, rotQX));
		m_quat = XMQuaternionNormalize(XMQuaternionMultiply(m_quat, rotQY));
		R = XMMatrixRotationQuaternion(m_quat);
	}
	m_prevMouse = m_currentMouse;

	
	if (GetAsyncKeyState(VK_LSHIFT) & 0x8000)
	{
		isAccelerate = true;
	}
	else
	{
		isAccelerate = false;
	}

	// 카메라 이동
	XMVECTOR pos = XMLoadFloat3(&m_position);
	if (GetAsyncKeyState('W') & 0x8000)	pos += forward * (m_moveSpeed + (m_moveSpeed * isAccelerate*0.7f));
	if (GetAsyncKeyState('A') & 0x8000)	pos -= right * (m_moveSpeed + (m_moveSpeed * isAccelerate * 0.7f));
	if (GetAsyncKeyState('S') & 0x8000)	pos -= forward * (m_moveSpeed + (m_moveSpeed * isAccelerate * 0.7f));
	if (GetAsyncKeyState('D') & 0x8000)	pos += right * (m_moveSpeed + (m_moveSpeed * isAccelerate * 0.7f));
	if (GetAsyncKeyState('Q') & 0x8000)	pos -= up * (m_moveSpeed + (m_moveSpeed * isAccelerate * 0.7f));
	if (GetAsyncKeyState('E') & 0x8000)	pos += up * (m_moveSpeed + (m_moveSpeed * isAccelerate * 0.7f));


	if (GetAsyncKeyState('O') & 0x8000)	m_moveSpeed -= 0.01f;
	if (GetAsyncKeyState('P') & 0x8000)	m_moveSpeed += 0.01f;


	// (1) 각 변환을 XMMATRIX로 생성
	XMMATRIX S = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	
	XMMATRIX T = XMMatrixTranslationFromVector(pos);
	
	// (2) 합성
	XMMATRIX world = S * R * T;

	XMStoreFloat4x4(&m_local, XMMatrixInverse(nullptr, world));
	XMStoreFloat3(&m_position,pos);

	DebugManager::GetInstance().PushDebugData(
		[this]()
		{
			static float f = 0.0f;
			static int counter = 0;
			ImGui::Begin("Camera Debuger");       
			ImGui::Text("이동 : W,A,S,D, L-Shift");
			ImGui::Text("이동속도 : O,P");			
			ImGui::Text("위치: X:%.2f, Y:%.2f, Z:%.2f", m_position.x, m_position.y, m_position.z);
			ImGui::Text("회전: X:%.2f, Y:%.2f, Z:%.2f", m_rotation.x, m_rotation.y, m_rotation.z);
			ImGui::Text("카메라 이동속도 : %.2f", m_moveSpeed + (m_moveSpeed * isAccelerate * 0.7f));

			ImGui::End();
		});
}