#include "Core_pch.h"
#include "Model.h"

Model::~Model() = default; // ¼Ò¸êÀÚ ±¸Çö
void Model::UpdateHierarchy(Node* node)
{
	auto string = node->GetName();
	if (node->GetParent())
	{
		auto local = node->GetLocalTM();
		auto parent = node->GetParent()->GetWorldTM();
		auto total = local * parent;
		node->SetWorldTM(total);
	}
	else
	{
		auto local = node->GetLocalTM();
		node->SetWorldTM(local);
	}

	for (auto& child : node->GetChildren())
	{
		if (child) UpdateHierarchy(child.get());
	}
}

