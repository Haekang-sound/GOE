#include "Engine_pch.h"
#include "RenderSystem.h"
#include "Scene.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "../GOE_Render/RenderObject.h"

void RenderSystem::Initialize()
{
	// 현재 렌더시스템은
	// 메쉬렌더러가 갖고 있는 메쉬의 id와 
	// 트랜스폼이 갖고 있을 행렬을 콘스탄트 버퍼로바꿔서 랜더러측이 생성하도록 한다.
	//for (int i = 0; i < GetScene()->GetMeshRendererManager()->GetComponents().size(); ++i)
	for (auto& meshRenderer: GetScene()->GetMeshRendererManager()->GetComponents())
	{
		RenderObjectData* data = new RenderObjectData();
		data->id = meshRenderer.GetID();
		data->meshID = meshRenderer.GetMeshID();
		data->meshIndex = meshRenderer.GetMeshIndex();
		data->modelID = meshRenderer.GetModelID();
		data->textureID = 
			GetScene()->GetMaterialManager()->GetComponentByOwner(meshRenderer.GetOwner()).GetTextureID();
		data->localTM =
			GetScene()->GetTransformManager()->GetComponentByOwner(meshRenderer.GetOwner()).GetLocalTM();

		m_context->renderer->AddRenderObejct(*data);
		delete data;
	}	
}

void RenderSystem::Update(double dTime)
{
	for(const auto& renderobj : m_context->renderer->GetRenderObjects())
	{
		if (renderobj.get()->IsVisible())
		{
			renderobj->SetLocalTM(
				GetScene()->GetTransformManager()->GetComponent(renderobj->GetID()).GetLocalTM());
		}
	}
}
