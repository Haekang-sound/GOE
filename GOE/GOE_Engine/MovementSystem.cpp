#include "Engine_pch.h"
#include "MovementSystem.h"

#include "Scene.h"
#include "Transform.h"
#include "MovementUnit.h"

void MovementSystem::Initialize()
{
}

void MovementSystem::Update()
{
	
	/// 트랜스폼컴포넌트를 가져와서 이하 업데이트를 진행한다.
	// 트랜스폼 컴포넌트를 어떻게 가져오지?? 
	for (const auto& movementunit : GetScene()->GetMovementUnitManager()->GetComponents())
	{				// 이동가능한 컴포넌트만 업데이트
		if (movementunit.IsMoveable())
		{
			auto& transform = GetScene()->GetTransformManager()->GetComponentByOwner(movementunit.GetOwner());

			/// 기저벡터를 뽑는 기능을 트랜스폼에 만들자
			GOE::FLoatVector4 right = transform.GetRightVector();
			GOE::FLoatVector4 up = transform.GetUpVector();
			GOE::FLoatVector4 forward = transform.GetForwardVector();

			// 카메라 이동
			GOE::FLoatVector4 pos = { 0,0,0,0 };
			if (GetAsyncKeyState(VK_UP) & 0x8000)	pos += forward * m_moveSpeed;
			if (GetAsyncKeyState(VK_LEFT) & 0x8000)	pos -= right * m_moveSpeed;
			if (GetAsyncKeyState(VK_DOWN) & 0x8000)	pos -= forward * m_moveSpeed;
			if (GetAsyncKeyState(VK_RIGHT) & 0x8000)pos += right * m_moveSpeed;

			/// 이거는 트랜스폼 안에서 해도 될듯? 
			// (1) 각 변환을 XMMATRIX로 생성
			GOE::Matrix4x4 S ={};
			GOE::Matrix4x4 R ={};
			GOE::Matrix4x4 T = {};

			 S = GOE::Matrix4x4::Identity();
			 R = GOE::Matrix4x4::Identity();
			 T = GOE::Matrix4x4::Translation(pos.x, pos.y, pos.z);

			// (2) 합성
			GOE::Matrix4x4 srt = S * R * T;

			/// 트랜스폼에 적용
			transform.SetLocalTM(transform.GetLocalTM() * srt);
		}
	}
}
