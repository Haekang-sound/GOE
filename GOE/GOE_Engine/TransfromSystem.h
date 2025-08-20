#pragma once
#include "ISystem.h"
class TransfromSystem : public ISystem
{
public:
	TransfromSystem(Scene* scene, GOE::EngineContext* context)
		: ISystem(scene, context)
	{
	}
	~TransfromSystem() = default;

public:
	void Initialize() override;
	void Update() override;

private:

};

