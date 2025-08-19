#include "Engine_pch.h"
#include "MeshRenderer.h"

MeshRenderer::MeshRenderer(size_t ownerID, size_t id)
	:IComponent(ownerID, id)
{
}
MeshRenderer::~MeshRenderer() = default;
