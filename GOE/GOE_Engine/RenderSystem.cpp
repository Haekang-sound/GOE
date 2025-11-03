#include "Engine_pch.h"
#include "RenderSystem.h"

#include "Scene.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "AnimationUnit.h"

#include "Material.h"
#include "../GOE_Render/RenderObject.h"

void RenderSystem::Initialize()
{
	// 현재 렌더시스템은
	// 메쉬렌더러가 갖고 있는 메쉬의 id와 
	// 트랜스폼이 갖고 있을 행렬을 콘스탄트 버퍼로바꿔서 랜더러측이 생성하도록 한다.
	for (auto& meshRenderer : GetScene()->GetMeshRendererManager()->GetComponents())
	{
		RenderObjectData* data = new RenderObjectData();
		data->id = meshRenderer.GetID();
		data->meshID = meshRenderer.GetMeshID();
		data->meshIndex = meshRenderer.GetMeshIndex();
		data->modelID = meshRenderer.GetModelID();
		data->textureID =
			GetScene()->GetMaterialManager()->GetComponentByOwner(meshRenderer.GetOwner())->GetTextureID();
		data->localTM =
			GetScene()->GetTransformManager()->GetComponentByOwner(meshRenderer.GetOwner())->GetLocalTM();
		data->isVisible = meshRenderer.IsVisible();
		if (GetScene()->GetAnimationUnitManager()->GetComponentByOwner(meshRenderer.GetOwner()))
		{
			data->isAnimated = GetScene()->GetAnimationUnitManager()->GetComponentByOwner(meshRenderer.GetOwner())->IsAnimated();
		}

		m_context->renderer->AddRenderObejct(*data);
		delete data;
	}
}

void RenderSystem::Update(double dTime)
{
	for (const auto& renderobj : m_context->renderer->GetRenderObjects())
	{

		if (renderobj.get()->IsVisible())
		{
			renderobj->SetLocalTM(GetScene()->GetTransformManager()->GetComponent(renderobj->GetID())->GetLocalTM());
			
			int bones = m_context->assetCore->GetMesh(renderobj.get()->GetMeshID())->GetBones().size();
			for (int i = 0; i < bones; ++i)
			{
				size_t nodeid = m_context->assetCore->GetMesh(renderobj.get()->GetMeshID())->GetBones()[i].get()->GetNode();
				size_t boneidx = m_context->assetCore->GetMesh(renderobj.get()->GetMeshID())->GetBones()[i].get()->GetBoneIndex();
				GOE::Matrix4x4 temp = m_context->assetCore->GetModel(renderobj.get()->GetModelID())->GetNodeFromMap(nodeid)->GetWorldTM();

				renderobj.get()->SetBoneTM(boneidx, temp);
			}
			for (int i = bones; i < 128; ++i)
			{
				renderobj.get()->SetBoneTM(i, GOE::Matrix4x4::Identity());
			}
		}		
	}
}
