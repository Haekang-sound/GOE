#include "Model.h"
#include "Core_pch.h"
#include "Model.h"
Model::~Model() = default; // ¼Ò¸êÀÚ ±¸Çö
void Model::UpdateHierarchy(Node* node)
{
	if (node->GetParent())
	{
		node->SetWorldTM(node->GetLocalTM()* node->GetParent()->GetWorldTM());
	}
	else
	{
		node->SetWorldTM(node->GetLocalTM());
	}

	for (auto& child : node->GetChildren())
	{
		UpdateHierarchy(child.get());
	}
}

