#include "Core_pch.h"

Model::~Model() = default; // ¼Ò¸êÀÚ ±¸Çö
void Model::UpdateHierarchy(Node* node)
{
	node->SetWorldTM();

	for (const auto& child : node->GetChildren())
	{
		UpdateHierarchy(child.get());
	}
}