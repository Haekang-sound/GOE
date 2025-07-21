#pragma once
#include <DirectXMath.h>
#include <windef.h>
using namespace DirectX;

class Camera
{
public:
	Camera();
	~Camera();

	void OnUpdate(POINT center);

public: 
	XMFLOAT4X4 m_local = {};
	// 스케일, 회전(라디안), 위치
	XMVECTOR m_quat = XMQuaternionIdentity();

	float m_rollSpeed = 0.01;
	float wheelValue = 0;
	int prev_wheelValue = 0;

	XMFLOAT3 m_position = {0, 0, -2};
	XMFLOAT3 m_rotation = {};
	XMFLOAT3 m_scale = {1,1,1};

	float m_moveSpeed = 0.25f;
	float cam_x = 0.f;
	float cam_y = 0.f;
	float cam_z = -2.f;

	float m_roatateSpeed = 0.001f;

	POINT m_currentMouse = {};
	POINT m_prevMouse = {};
	bool m_keydown = false;

public: 
	inline XMMATRIX GetViewTransform()
	{
		return XMLoadFloat4x4(&m_local);
	}



};

