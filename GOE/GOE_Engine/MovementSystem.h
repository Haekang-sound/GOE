#pragma once
#include "ISystem.h"
class MovementSystem : public ISystem
{
public:
	MovementSystem(Scene* scene, GOE::EngineContext* context)
		: ISystem(scene, context){}
	~MovementSystem() = default;

public:
	void Initialize() override;
	void Update(double dTime) override;
	void DebugUpdate(double dTime) override;
	
private:
	float m_moveSpeed = 10;
};

