#pragma once
#include <DirectXMath.h>
#include <windef.h>
using namespace DirectX;

class Camera
{
public:
	Camera(HWND hWnd);
	Camera();
	~Camera();

	void OnUpdate();

private:
	HWND m_hWnd = nullptr;
	XMFLOAT4X4 m_local = {};
	// 스케일, 회전(라디안), 위치
	XMVECTOR m_quat = XMQuaternionIdentity();

	double m_rollSpeed = 0.01;
	float wheelValue = 0;
	int prev_wheelValue = 0;

	XMFLOAT3 m_position = {0.f, 15.f, -36.f};
	XMFLOAT3 m_rotation = {0.22f,0.f,0.f};
	XMFLOAT3 m_scale = {1.f,1.f,1.f};

	float m_moveSpeed = 0.25f;
	bool isAccelerate = false;
	float cam_x = 0.f;
	float cam_y = 0.f;
	float cam_z = -2.f;

	float m_roatateSpeed = 0.001f;

	POINT m_currentMouse = {};
	POINT m_prevMouse = {};
	bool m_keydown = false;

public: 
	inline XMMATRIX GetViewTransform(){	return XMLoadFloat4x4(&m_local);}
	inline XMFLOAT3 GetPosition() const { return m_position; }
};

