#include "Engine_pch.h"
#include "MovementSystem.h"
#include "InputManager.h"

#include "Scene.h"
#include "Transform.h"
#include "MovementUnit.h"
#include "../GOE_Editor/DebugManager.h"
#include "../Imgui/imgui.h"

void MovementSystem::Initialize()
{

}

void MovementSystem::Update(double dTime)
{
	// 이동 속도 조절
	if (GOE::InputManager::GetInstance().GetButton('N')) m_moveSpeed -= 1.f * dTime;
	if (GOE::InputManager::GetInstance().GetButton('M')) m_moveSpeed += 1.f * dTime;

	// 가속 (Shift)
	float currentSpeed = m_moveSpeed;
	if (GOE::InputManager::GetInstance().GetButton(VK_LSHIFT)) currentSpeed *= 2.5f;

	for (const auto& movementunit : GetScene()->GetMovementUnitManager()->GetComponents())
	{
		// 이동가능한 컴포넌트만 업데이트
		if (movementunit.IsMoveable())
		{
			auto transform = GetScene()->GetTransformManager()->GetComponentByOwner(movementunit.GetOwner());

			// 1. 회전 처리
			GOE::FLoatVector3 rotation = transform->GetRotation();

			if (GOE::InputManager::GetInstance().GetMouseButton(GOE::MouseButton::Right))
			{
				POINT mouseDelta = GOE::InputManager::GetInstance().GetMouseDelta();
				float rotateSpeed = 1.0f * (float)dTime;

				// Yaw (World Y축 기준)
				rotation.y += mouseDelta.x * rotateSpeed;
				// Pitch (Local X축 기준)
				rotation.x += mouseDelta.y * rotateSpeed;

				transform->SetRotation(rotation); // LocalTM 갱신됨
			}

			// 키보드 회전
			if (GOE::InputManager::GetInstance().GetButton(VK_LEFT)) { rotation.y -= 2.0f * dTime; transform->SetRotation(rotation); }
			if (GOE::InputManager::GetInstance().GetButton(VK_RIGHT)) { rotation.y += 2.0f * dTime; transform->SetRotation(rotation); }
			if (GOE::InputManager::GetInstance().GetButton(VK_UP)) { rotation.x -= 2.0f * dTime; transform->SetRotation(rotation); }
			if (GOE::InputManager::GetInstance().GetButton(VK_DOWN)) { rotation.x += 2.0f * dTime; transform->SetRotation(rotation); }

			// 2. 이동 처리 (회전된 기저벡터 기준)
			GOE::FLoatVector4 right = transform->GetRightVector();
			GOE::FLoatVector4 forward = transform->GetForwardVector();
			// Free Camera는 보는 방향(Forward)으로 이동하므로 Up 벡터 대신 Forward를 사용하거나,
			// 수직 상승/하강을 위해 World Up을 사용할 수 있음. 여기선 Local Up(카메라 상단) 사용.
			GOE::FLoatVector4 up = transform->GetUpVector();

			GOE::FLoatVector3 pos = transform->GetPosition();
			GOE::FLoatVector3 deltaPos = { 0,0,0 };

			if (GOE::InputManager::GetInstance().GetButton('W'))	deltaPos += forward * currentSpeed * dTime;
			if (GOE::InputManager::GetInstance().GetButton('S'))	deltaPos -= forward * currentSpeed * dTime;
			if (GOE::InputManager::GetInstance().GetButton('A'))	deltaPos -= right * currentSpeed * dTime;
			if (GOE::InputManager::GetInstance().GetButton('D'))	deltaPos += right * currentSpeed * dTime;
			if (GOE::InputManager::GetInstance().GetButton('Q'))	deltaPos -= up * currentSpeed * dTime;
			if (GOE::InputManager::GetInstance().GetButton('E'))	deltaPos += up * currentSpeed * dTime;

			pos += deltaPos;
			transform->SetPosition(pos);
		}
	}
}

void MovementSystem::DebugUpdate(double dTime)
{
	DebugManager::GetInstance().PushDebugData(
		[this]()
		{
			ImGui::Begin("Movement System");
			ImGui::Text("Move: W,A,S,D, Q,E (Shift: Fast)");
			ImGui::Text("Rotate: Mouse Right Drag or Arrows");
			ImGui::Text("Speed Control: N(-), M(+)");
			ImGui::Text("Current Speed : %.2f", m_moveSpeed);
			ImGui::End();
		});

}
