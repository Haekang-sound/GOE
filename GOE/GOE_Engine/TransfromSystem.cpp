#include "Engine_pch.h"
#include "TransfromSystem.h"
#include "Scene.h"
#include "Transform.h"

void TransfromSystem::Initialize()
{
}

void TransfromSystem::Update()
{
	for (const auto& transform : GetScene()->GetTransformManager()->GetComponents())
	{


	}
}
