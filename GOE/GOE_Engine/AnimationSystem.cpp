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
	/// 프레임드랍 원인 찾기
	///auto start = std::chrono::high_resolution_clock::now();

	for (auto& animationUnit : GetScene()->GetAnimationUnitManager()->GetComponents())
	{
		/// 매쉬와 모델을 입력받는 편이 좋을지도
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

			///-------------------------------캐싱테스트중--------
			int boneCount = m_anim->GetBoneAnimation().size();
			if (animationUnit.m_cachedBoneIndices.size() != boneCount)
			{
				animationUnit.m_cachedBoneIndices.resize(boneCount, 0);
			}

			for (int i = 0; i < boneCount; ++i)
			{
				m_boneAnim = m_anim->GetBoneAnimation()[i].get();

				Node* currentNode = m_model->GetNodeFromMap(m_boneAnim->GetID());
				if (currentNode)
				{
					// [수정] 캐시 인덱스를 참조(&)로 넘겨줍니다.
					currentNode->SetLocalTM(
						m_boneAnim->InterpolateSRT(
							animationUnit.m_normalizedTick,
							animationUnit.m_cachedBoneIndices[i] // <- 여기!
						)
					);
				}
			}
			//-------------------------------캐싱테스트중--------


			for (int i = 0; i < m_anim->GetBoneAnimation().size(); ++i)
			{
				// 3. 현재 tick에 해당하는 프레임 인덱스들과 각 프레임의 대한 가중치를 구한다.
				m_boneAnim = m_anim->GetBoneAnimation()[i].get();

				Node* currentNode = m_model->GetNodeFromMap(m_boneAnim->GetID());
				if (currentNode)
				{
					currentNode->SetLocalTM(m_boneAnim->InterpolateSRT(animationUnit.m_normalizedTick));// *currentNode->GetNodePositionMatrix());
				}
			}
		}
		m_model->UpdateNodeHierarchy();
	}
	auto end = std::chrono::high_resolution_clock::now();
	/// 프레임드랍 원인 찾기
	//std::chrono::duration<double, std::milli> ms = end - start;
	//if (ms.count() > 1.0) // 1ms 이상 걸리면 경고
	//{
	//	char buf[100];
	//	sprintf_s(buf, "Anim Update: %f ms\n", ms.count());
	//	OutputDebugStringA(buf);
	//}
}


