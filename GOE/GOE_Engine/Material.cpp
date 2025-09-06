#include "Engine_pch.h"
#include "Material.h"

Material::Material(size_t ownerID, size_t id)
	:IComponent(ownerID, id)
{
}
Material::~Material() = default;
