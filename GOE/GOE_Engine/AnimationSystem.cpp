#include "Engine_pch.h"
#include "AnimationSystem.h"

#include "Scene.h"
#include "AnimationUnit.h"

#include "DebugManager.h"
#include "../Imgui/imgui.h"
#include "../GOE_Render/RenderObject.h"
#include "MeshRenderer.h"

void AnimationSystem::Initialize() {}

int AnimationSystem::GetFrameIndexFromTime(float animTime, float duration, int frameCount)
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

void AnimationSystem::Update(double dTime)
{
	for (const auto& animationUnit : GetScene()->GetAnimationUnitManager()->GetComponents())
	{
		if (animationUnit.IsAnimated())
		{
			size_t meshID = GetScene()->GetMeshRendererManager()->GetComponentByOwner(animationUnit.GetOwner())->GetMeshID();
			size_t modelID = GetScene()->GetMeshRendererManager()->GetComponentByOwner(animationUnit.GetOwner())->GetModelID();
			int bones = m_context->assetCore->GetMesh(meshID)->GetBones().size();

			// 애니메이션을 해쉬로저장할떄 단순 이름으로 저장하면 안될것 같다 왜냐면 애니메이션 이름이 죄다 mixamo.com 이기때문에
			Animation* temp = m_context->assetCore->GetAnimation(animationUnit.GetAnimationHash());
			Model* tempM = m_context->assetCore->GetModel(modelID);

			static float elapsedTime = 0.0f;
			elapsedTime += (float)dTime;

			float ticksPerSecond = temp->GetTicksPerSecond(); // 예: 30
			float duration = temp->GetDuration();              // 예: 559

			float timeInTicks = elapsedTime * ticksPerSecond; // 초 → tick 변환
			float animTime = fmod(timeInTicks, duration);     // 루프 재생
			int keyframeCount = 155; // 예: 155
			int frameIndex = GetFrameIndexFromTime(animTime, duration, keyframeCount);

			// 1. 애니메이션은 순회하면서 모델 내부의 노드를 업데이트한다.
			for (int i = 0; i < temp->GetBoneAnimation().size(); ++i)
			{
				BoneAnimation* boneAnim = temp->GetBoneAnimation()[i].get();
				Node* currentNode = tempM->GetNodeFromMap(boneAnim->GetID());
				if (currentNode)
				{
					currentNode->SetLocalTM(boneAnim->GetSRMatrix(frameIndex) * currentNode->GetNodePositionMatrix());
				}
			}

			tempM->UpdateNodeHierarchy();
		}
	}


}


