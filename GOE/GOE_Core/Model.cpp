#include "Core_pch.h"

Model::~Model() = default; // ¼Ò¸êÀÚ ±¸Çö
void Model::UpdateHierarchy(Node* node)
{
	if (node->GetParent())
	{
		node->SetWorldTM(node->GetParent()->GetWorldTM() * node->GetLocalTM());
	}
	else
	{
		node->SetWorldTM(node->GetLocalTM());
	}

	for (const auto& child : node->GetChildren())
	{
		UpdateHierarchy(child.get());
	}
}