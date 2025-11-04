#include "Engine_pch.h"
#include "AnimationSystem.h"

#include "Scene.h"
#include "AnimationUnit.h"

#include "DebugManager.h"
#include "../Imgui/imgui.h"
#include "../GOE_Render/RenderObject.h"
#include "MeshRenderer.h"

void AnimationSystem::Initialize() {}

int AnimationSystem::GetFrameIndexFromTime(double animTime, double duration, int frameCount)
{
	// 1️⃣ 방어 코드
	// 프레임이 1개 이하거나, duration이 0이면 무조건 0번째 프레임
	if (frameCount <= 1 || duration <= 0.0f)
	{
		return 0;
	}

	// 2️⃣ 현재 애니메이션의 진행률을 구함 (0.0 ~ 1.0)
	// animTime은 tick 단위, duration도 tick 단위이므로 단순 비율 계산 가능
	float normalizedTime = animTime / duration;

	// 3️⃣ 1.0을 넘어가면 루프 애니메이션이므로 다시 0~1로 돌림
	// (fmod를 사용하면 animTime이 duration을 초과해도 정상 반복)
	normalizedTime = fmod(normalizedTime, 1.0f);

	// 4️⃣ 음수 보정 (혹시 animTime이 음수가 될 경우 대비)
	if (normalizedTime < 0.0f)
	{
		normalizedTime += 1.0f;
	}

	// 5️⃣ 진행률에 전체 프레임 개수를 곱해서 실제 인덱스로 변환
	float frameFloat = normalizedTime * frameCount;

	// 6️⃣ 실수를 정수 인덱스로 변환
	int frameIndex = static_cast<int>(frameFloat);

	// 7️⃣ 마지막 프레임을 넘어가지 않게 보정
	if (frameIndex >= frameCount)
	{
		frameIndex = frameCount - 1;
	}

	// 8️⃣ 계산된 프레임 인덱스를 반환
	return frameIndex;
}

/// <summary>
/// 본애니메이션과 키프레임을 입력받고
/// 보간된 변환 행렬을 반환한다.
/// </summary>
/// <param name="boneAnim">본애니메이션</param>
/// <param name="animTime">현재 틱</param>
/// <returns></returns>
GOE::Matrix4x4 AnimationSystem::InterpolateTransform(BoneAnimation* boneAnim, double noramlizedTime)
{
	int scaleKeyframeCount = m_boneAnim->GetScaleCount();
	int rotaionKeyframeCount = m_boneAnim->GetRotationCount();
	int positionKeyframeCount = m_boneAnim->GetPositionCount();




	return GOE::Matrix4x4();
}

void AnimationSystem::Update(double dTime)
{
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

			// 현재 루트모션이 제거된 애니메이션만 사용할 수 있음
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


}


