#include "Core_pch.h"
#include "BoneAnimation.h"

BoneAnimation::~BoneAnimation() = default;

GOE::Matrix4x4 BoneAnimation::InterpolateScale(double normaliedTime)
{
	if (m_scales.size() == 1)
	{
		return m_scales[0].value.ToScaleMatrix();
	}
	int indexA = FindKeyIndex<VectorKeyFrame>(normaliedTime, m_scales);
	int indexB = indexA + 1;
	// --- ✅ 수정 시작 ---
	float timeA = m_scales[indexA].time;
	float timeB = m_scales[indexB].time;
	float timeSpan = timeB - timeA;

	// 0으로 나누기 방지
	if (timeSpan <= 0.0f)
	{
		return m_scales[indexA].value.ToScaleMatrix();
	}

	float factor = (normaliedTime - timeA) / timeSpan;
	// --- ✅ 수정 끝 ---
	GOE::FLoatVector3 scaleA = m_scales[indexA].value;
	GOE::FLoatVector3 scaleB = m_scales[indexB].value;
	GOE::FLoatVector3 finalScale = scaleA;
	finalScale += (scaleB - scaleA) * factor;
	return finalScale.ToScaleMatrix();


}

GOE::Matrix4x4 BoneAnimation::InterpolatePosition(double normaliedTime)
{
	if (m_positions.size() == 1)
	{
		return m_positions[0].value.ToTranslationMatrix();
	}
	int indexA = FindKeyIndex<VectorKeyFrame>(normaliedTime, m_positions);
	int indexB = indexA + 1;
	float timeA = m_positions[indexA].time;
	float timeB = m_positions[indexB].time;
	float factor = (normaliedTime - timeA) / (timeB - timeA);
	GOE::FLoatVector3 posA = m_positions[indexA].value;
	GOE::FLoatVector3 posB = m_positions[indexB].value;
	GOE::FLoatVector3 finalPos = posA;
	finalPos += (posB - posA) * factor;
	return finalPos.ToTranslationMatrix();
}

GOE::Matrix4x4 BoneAnimation::InterpolateQuatanion(double normaliedTime)
{
	// --- 방어 코드 1: 키가 1개일 때 ---
	if (m_quatanions.size() == 1)
	{
		// ✅ FIX 2: 올바른 함수 이름 사용
		return m_quatanions[0].value.ToRotationMatrix();
	}

	// --- 경계 키프레임 탐색 ---
	int indexA = FindKeyIndex<QuatKeyFrame>(normaliedTime, m_quatanions);
	int indexB = indexA + 1;

	// --- ✅ FIX 3: 0으로 나누기(NaN) 방지 ---
	float timeA = m_quatanions[indexA].time;
	float timeB = m_quatanions[indexB].time;
	float timeSpan = timeB - timeA;

	// 시간이 같거나 역전되면 보간하지 않고 KeyA의 값을 반환
	if (timeSpan <= 0.0f)
	{
		// ✅ FIX 2: 올바른 함수 이름 사용
		return m_quatanions[indexA].value.ToRotationMatrix();
	}

	// --- 보간 계수(Factor) 계산 ---
	float factor = (normaliedTime - timeA) / timeSpan;

	// --- ✅ FIX 1: Slerp (구면 선형 보간) 사용 ---
	GOE::FLoatVector4 quatA = m_quatanions[indexA].value;
	GOE::FLoatVector4 quatB = m_quatanions[indexB].value;

	// Lerp 대신 Slerp 호출
	GOE::FLoatVector4 finalQuat = GOE::FLoatVector4::Slerp(quatA, quatB, factor);

	// ✅ FIX 2: 올바른 함수 이름 사용
	return finalQuat.ToRotationMatrix();
}