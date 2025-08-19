#include "Engine_pch.h"
#include "RenderSystem.h"
#include "Scene.h"
#include "Transform.h"
#include "MeshRenderer.h"

void RenderSystem::Initialize()
{
	// 현재 렌더시스템은
	// 메쉬렌더러가 갖고 있는 메쉬의 id와 
	// 트랜스폼이 갖고 있을 행렬을 콘스탄트 버퍼로바꿔서 랜더러측이 생성하도록 한다.

	for (int i = 0; i < GetScene()->GetMeshRendererManager()->GetComponents().size(); ++i)
	{
		RenderObjectData* data = new RenderObjectData();
		data->id = i;
		data->meshID = GetScene()->GetMeshRendererManager()->GetComponents()[i].GetMeshID();
		data->meshIndex = GetScene()->GetMeshRendererManager()->GetComponents()[i].GetMeshIndex();
		data->modelID = GetScene()->GetMeshRendererManager()->GetComponents()[i].GetModelID();
		data->localTM = 
			GetScene()->GetTransformManager()->GetComponents()[i].GetLocalTM();

		GetScene()->GetTransformManager()->GetComponents()[i].GetLocalTM();
		GetScene()->GetMeshRendererManager()->GetComponents()[i].GetMeshID();

		m_context->renderer->AddRenderObejct(*data);
		delete data;
	}

	
}

void RenderSystem::Update()
{

}
