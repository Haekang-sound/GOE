#pragma once
#include "ISystem.h"
class AnimationSystem : public ISystem
{
public:
	AnimationSystem(Scene* scene, GOE::EngineContext* context)
		: ISystem(scene, context)
	{
	}
	~AnimationSystem() = default;

public:
	void Initialize() override;
	void Update(double dTime) override;

protected:
	int GetFrameIndexFromTime(double animTime, double duration, int frameCount);
	GOE::Matrix4x4 InterpolateTransform(BoneAnimation* boneAnim, double noramlizedTime);

protected:
	//double totalTime = 0.0f;
	//double ticksPerSecond = 0.0f;
	//double duration = 0.0f;
	//double m_totalTick = 0.0f;
	//double m_noramlizedTick = 0.0f;

	Animation* m_anim = nullptr;
	Model* m_model = nullptr;
	BoneAnimation* m_boneAnim = nullptr;
};