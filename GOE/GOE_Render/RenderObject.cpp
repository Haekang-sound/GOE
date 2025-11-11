#include "Renderer_pch.h"
#include "RenderObject.h"

RenderObject::RenderObject(RenderObjectData& data)
	: m_id(data.id), m_name(data.name),
	m_modelID(data.modelID), m_meshIndex(data.meshIndex),
	m_meshID(data.meshID), m_textureID(data.textureID), m_localTM(data.localTM), m_isVisible(data.isVisible), m_isAnimated(data.isAnimated)
{
	for (int i = 0; i < 128; ++i)
	{
		m_boneTM[i] = data.boneTM[i];
	}
}

RenderObject::~RenderObject() = default;

