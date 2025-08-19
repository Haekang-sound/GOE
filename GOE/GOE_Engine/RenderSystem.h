#pragma once
#include "ISystem.h"

class RenderSystem : public ISystem
{
public:
	RenderSystem(Scene* scene, GOE::EngineContext* context)
		: ISystem(scene, context){}
	~RenderSystem() = default;

public: 
	void Initialize() override;
	void Update() override;

private:
	
};

