#pragma once
#include "ISystem.h"
class AnimationSystem :	public ISystem
{
public:
	AnimationSystem(Scene* scene, GOE::EngineContext* context)
		: ISystem(scene, context){}
	~AnimationSystem() = default;

public:
	void Initialize() override;
	void Update(double dTime) override;

private:
	int GetFrameIndexFromTime(float animTime, float duration, int frameCount);

};