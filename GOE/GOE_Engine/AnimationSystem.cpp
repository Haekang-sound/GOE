#include "Engine_pch.h"
#include "AnimationSystem.h"

#include "Scene.h"
#include "AnimationUnit.h"

#include "DebugManager.h"
#include "../Imgui/imgui.h"
#include "../GOE_Render/RenderObject.h"
#include "MeshRenderer.h"

void AnimationSystem::Initialize() {}

void AnimationSystem::Update(double dTime)
{
	for (auto& animationUnit : GetScene()->GetAnimationUnitManager()->GetComponents())
	{
		size_t modelID = GetScene()->GetMeshRendererManager()->GetComponentByOwner(animationUnit.GetOwner())->GetModelID();
		m_model = m_context->assetCore->GetModel(modelID);
		// 애니메이션을 해쉬로저장할떄 단순 이름으로 저장하면 안될것 같다 왜냐면 애니메이션 이름이 죄다 mixamo.com 이기때문에

		if (animationUnit.IsAnimated())
		{
			size_t meshID = GetScene()->GetMeshRendererManager()->GetComponentByOwner(animationUnit.GetOwner())->GetMeshID();
			m_anim = m_context->assetCore->GetAnimation(animationUnit.GetAnimationHash());

			// 1. 시간을 누적한다.
			animationUnit.m_totalTime += dTime;

			// 2. 누적된 시간을 바탕으로 현재 tick(정규화된 시간)을 구한다.
			animationUnit.m_ticksPerSecond = m_anim->GetTicksPerSecond();
			animationUnit.m_duration = m_anim->GetDuration();
			animationUnit.m_totalTick = animationUnit.m_totalTime * animationUnit.m_ticksPerSecond;
			// 정규화된 시간
			animationUnit.m_normalizedTick = fmod(animationUnit.m_totalTick, animationUnit.m_duration);

			int boneCount = m_anim->GetBoneAnimation().size();
			// 캐시벡터사이즈가 본갯수와 다를 경우
			if (animationUnit.m_cachedBoneIndices.size() != boneCount)
			{
				// 벡터를 본갯수만큼 리사이즈 한다.
				animationUnit.m_cachedBoneIndices.resize(boneCount, {0,0,0});
			}

			//본을 순회한다.
			for (int i = 0; i < boneCount; ++i)
			{
				// 현재 본인덱스를 통해 본애니메이션을 찾아온다
				m_boneAnim = m_anim->GetBoneAnimation()[i].get();

				// 본애니메이션과 연결된 NODE를 가져온다.
				Node* currentNode = m_model->GetNodeFromMap(m_boneAnim->GetID());
				
				// 노드가 존재할경우
				if (currentNode)
				{
					// 현재노드에 맞는 localTM을 설정한다.
					currentNode->SetLocalTM(
						m_boneAnim->InterpolateSRT(
							animationUnit.m_normalizedTick,// 현재 프레임의 정규화된 시간
							animationUnit.m_cachedBoneIndices[i] // 본의 마지막인덱스를 저장, 참조전달하는 부분, 여기부터 검사하면 효율이 올라가니까
						)
					);
				}
			}
		}

		m_model->UpdateNodeHierarchy();
	}
}


