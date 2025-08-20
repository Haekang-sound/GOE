#pragma once
#include "ISystem.h"
class MovementSystem : public ISystem
{
public:
	MovementSystem(Scene* scene, GOE::EngineContext* context)
		: ISystem(scene, context)
	{
	}
	~MovementSystem() = default;

public:
	void Initialize() override;
	void Update() override;

private:
	float m_moveSpeed = 10; // 이동 속도
};

