#include "Engine_pch.h"
#include "RenderSystem.h"

#include "Scene.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "AnimationUnit.h"
#include "CameraComponent.h" // 추가

#include "Material.h"

void RenderSystem::Initialize()
{

}

void RenderSystem::Update(double dTime)
{
	auto meshRenderers = GetScene()->GetMeshRendererManager()->GetComponents();
	auto renderer = m_context->renderer;
	m_renderObjects.clear();

	// 카메라 업데이트 로직 추가
	auto cameras = GetScene()->GetCameraManager()->GetComponents();
	for (auto& camera : cameras)
	{
		if (camera.IsActive()) // 활성화된 카메라만 사용 (여러 개일 경우 마지막 것 사용 or 우선순위 필요)
		{
			auto transform = GetScene()->GetTransformManager()->GetComponentByOwner(camera.GetOwner());
			if (transform)
			{
				GOE::Matrix4x4 worldMat = transform->GetLocalTM(); // World TM
				GOE::FLoatVector3 pos = transform->GetPosition();

				renderer->SetCameraData(
					camera.GetFOV(),
					camera.GetAspectRatio(),
					camera.GetNearZ(),
					camera.GetFarZ(),
					worldMat,
					pos
				);
			}
			break; // 첫 번째 활성 카메라만 처리하고 종료		
		}
	}


	for (auto& meshRenderer : meshRenderers)
	{
		if (!meshRenderer.IsVisible()) continue;
		m_renderObjects.emplace_back(meshRenderer.GetID());
		m_renderObjects.back().SetMeshID(meshRenderer.GetMeshID());
		m_renderObjects.back().SetMeshIndex(meshRenderer.GetMeshIndex());
		m_renderObjects.back().SetModelID(meshRenderer.GetModelID());
		m_renderObjects.back().SetTextureID(
			GetScene()->GetMaterialManager()->GetComponentByOwner(meshRenderer.GetOwner())->GetTextureID());
		m_renderObjects.back().SetLocalTM(
			GetScene()->GetTransformManager()->GetComponentByOwner(meshRenderer.GetOwner())->GetLocalTM());

		size_t bones = m_context->assetCore->GetMesh(meshRenderer.GetMeshID())->GetBones().size();
		for (int i = 0; i < bones; ++i)
		{
			size_t nodeid = m_context->assetCore->GetMesh(meshRenderer.GetMeshID())->GetBones()[i].get()->GetNode();
			size_t boneidx = m_context->assetCore->GetMesh(meshRenderer.GetMeshID())->GetBones()[i].get()->GetBoneIndex();
			GOE::Matrix4x4 temp = m_context->assetCore->GetModel(meshRenderer.GetModelID())->GetNodeFromMap(nodeid)->GetWorldTM();

			m_renderObjects.back().SetBoneTM(boneidx, temp);
		}
		for (size_t i = bones; i < 128; ++i)
		{
			m_renderObjects.back().SetBoneTM(i, GOE::Matrix4x4::Identity());
		}

	}

	renderer->ReceiveRenderObejcts(std::move(m_renderObjects));
}
