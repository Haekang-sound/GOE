#include "Engine_pch.h"
#include "RenderSystem.h"

#include "Scene.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "AnimationUnit.h"

#include "Material.h"

void RenderSystem::Initialize()
{
	
}

void RenderSystem::Update(double dTime)
{
	// 랜더오브젝트를 순회합니다.
	auto meshRenderers = GetScene()->GetMeshRendererManager()->GetComponents();
	auto renderer = m_context->renderer;
	m_renderObjects.clear();

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

	//for (const auto& renderobj : m_renderObjects)
	//{

	//	if (renderobj->IsVisible())
	//	{
	//		renderobj->SetLocalTM(GetScene()->GetTransformManager()->GetComponent(renderobj->GetID())->GetLocalTM());
	//		
	//		size_t bones = m_context->assetCore->GetMesh(renderobj->GetMeshID())->GetBones().size();
	//		for (int i = 0; i < bones; ++i)
	//		{
	//			size_t nodeid = m_context->assetCore->GetMesh(renderobj->GetMeshID())->GetBones()[i].get()->GetNode();
	//			size_t boneidx = m_context->assetCore->GetMesh(renderobj->GetMeshID())->GetBones()[i].get()->GetBoneIndex();
	//			GOE::Matrix4x4 temp = m_context->assetCore->GetModel(renderobj->GetModelID())->GetNodeFromMap(nodeid)->GetWorldTM();

	//			renderobj->SetBoneTM(boneidx, temp);
	//		}
	//		for (size_t i = bones; i < 128; ++i)
	//		{
	//			renderobj->SetBoneTM(i, GOE::Matrix4x4::Identity());
	//		}
	//	}		
	//}
}
