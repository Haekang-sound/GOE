#pragma once
#include "ISystem.h"
#include "../GOE_Render/RenderObject.h"

class RenderObject;

class RenderSystem : public ISystem
{
public:
	RenderSystem(Scene* scene, GOE::EngineContext* context)
		: ISystem(scene, context){}
	~RenderSystem() = default;

public: 
	void Initialize() override;
	void Update(double dTime) override;

protected:
	std::vector<RenderObject> m_renderObjects;
};

