#pragma once
#include "ISystem.h"
class AnimationSystem : public ISystem
{
public:
	AnimationSystem(Scene* scene, GOE::EngineContext* context)
		: ISystem(scene, context){}

public:
	void Initialize() override;
	void Update(double dTime) override;

protected:
	Animation* m_anim = nullptr;
	Model* m_model = nullptr;
	BoneAnimation* m_boneAnim = nullptr;
};