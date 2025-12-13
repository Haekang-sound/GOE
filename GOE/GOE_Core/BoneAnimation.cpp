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

	double timeA = m_scales[indexA].time;
	double timeB = m_scales[indexB].time;
	double timeSpan = timeB - timeA;

	if (timeSpan <= 0.0f)
	{
		return m_scales[indexA].value.ToScaleMatrix();
	}

	double factor = (normaliedTime - timeA) / timeSpan;

	GOE::FLoatVector3 scaleA = m_scales[indexA].value;
	GOE::FLoatVector3 scaleB = m_scales[indexB].value;
	GOE::FLoatVector3 finalScale = scaleA;
	finalScale += (scaleB - scaleA) * static_cast<float>(factor);
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

	double timeA = m_positions[indexA].time;
	double timeB = m_positions[indexB].time;
	double timeSpan = timeB - timeA;

	if (timeSpan <= 0.0f)
	{
		return m_positions[indexA].value.ToTranslationMatrix();
	}

	double factor = (normaliedTime - timeA) / timeSpan;

	GOE::FLoatVector3 posA = m_positions[indexA].value;
	GOE::FLoatVector3 posB = m_positions[indexB].value;
	GOE::FLoatVector3 finalPos = posA;

	finalPos += (posB - posA) * static_cast<float>(factor);

	return finalPos.ToTranslationMatrix();
}
GOE::Matrix4x4 BoneAnimation::InterpolateQuatanion(double normaliedTime)
{
	if (m_quatanions.size() == 1)
	{
		return m_quatanions[0].value.ToRotationMatrix();
	}

	int indexA = FindKeyIndex<QuatKeyFrame>(normaliedTime, m_quatanions);
	int indexB = indexA + 1;

	double timeA = m_quatanions[indexA].time;
	double timeB = m_quatanions[indexB].time;
	double timeSpan = timeB - timeA;

	if (timeSpan <= 0.0f)
	{
		return m_quatanions[indexA].value.ToRotationMatrix();
	}

	double factor = (normaliedTime - timeA) / timeSpan;

	GOE::FLoatVector4 quatA = m_quatanions[indexA].value;
	GOE::FLoatVector4 quatB = m_quatanions[indexB].value;

	// Lerp 대신 Slerp 호출
	GOE::FLoatVector4 finalQuat = GOE::FLoatVector4::Slerp(quatA, quatB, static_cast<float>(factor));

	// ✅ FIX 2: 올바른 함수 이름 사용
	return finalQuat.ToRotationMatrix();
}

GOE::Matrix4x4 BoneAnimation::InterpolateScale(double normaliedTime, int& cacheIndex)
{
	if (m_scales.size() == 1)
	{
		return m_scales[0].value.ToScaleMatrix();
	}
	int indexA = FindKeyIndex<VectorKeyFrame>(normaliedTime, m_scales, cacheIndex);
	int indexB = indexA + 1;

	double timeA = m_scales[indexA].time;
	double timeB = m_scales[indexB].time;
	double timeSpan = timeB - timeA;

	if (timeSpan <= 0.0f)
	{
		return m_scales[indexA].value.ToScaleMatrix();
	}

	double factor = (normaliedTime - timeA) / timeSpan;

	GOE::FLoatVector3 scaleA = m_scales[indexA].value;
	GOE::FLoatVector3 scaleB = m_scales[indexB].value;
	GOE::FLoatVector3 finalScale = scaleA;
	finalScale += (scaleB - scaleA) * static_cast<float>(factor);
	return finalScale.ToScaleMatrix();


}
GOE::Matrix4x4 BoneAnimation::InterpolatePosition(double normaliedTime, int& cacheIndex)
{
	if (m_positions.size() == 1)
	{
		return m_positions[0].value.ToTranslationMatrix();
	}

	int indexA = FindKeyIndex<VectorKeyFrame>(normaliedTime, m_positions, cacheIndex);
	int indexB = indexA + 1;

	double timeA = m_positions[indexA].time;
	double timeB = m_positions[indexB].time;
	double timeSpan = timeB - timeA;

	if (timeSpan <= 0.0f)
	{
		return m_positions[indexA].value.ToTranslationMatrix();
	}

	double factor = (normaliedTime - timeA) / timeSpan;

	GOE::FLoatVector3 posA = m_positions[indexA].value;
	GOE::FLoatVector3 posB = m_positions[indexB].value;
	GOE::FLoatVector3 finalPos = posA;

	finalPos += (posB - posA) * static_cast<float>(factor);

	return finalPos.ToTranslationMatrix();
}

GOE::Matrix4x4 BoneAnimation::InterpolateQuatanion(double normaliedTime, int& cacheIndex)
{
	if (m_quatanions.size() == 1)
	{
		return m_quatanions[0].value.ToRotationMatrix();
	}

	int indexA = FindKeyIndex<QuatKeyFrame>(normaliedTime, m_quatanions, cacheIndex);
	int indexB = indexA + 1;

	double timeA = m_quatanions[indexA].time;
	double timeB = m_quatanions[indexB].time;
	double timeSpan = timeB - timeA;

	if (timeSpan <= 0.0f)
	{
		return m_quatanions[indexA].value.ToRotationMatrix();
	}

	double factor = (normaliedTime - timeA) / timeSpan;

	GOE::FLoatVector4 quatA = m_quatanions[indexA].value;
	GOE::FLoatVector4 quatB = m_quatanions[indexB].value;

	// Lerp 대신 Slerp 호출
	GOE::FLoatVector4 finalQuat = GOE::FLoatVector4::Slerp(quatA, quatB, static_cast<float>(factor));

	// ✅ FIX 2: 올바른 함수 이름 사용
	return finalQuat.ToRotationMatrix();
}


