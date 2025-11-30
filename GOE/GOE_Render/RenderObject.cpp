#include "Renderer_pch.h"
#include "RenderObject.h"

RenderObject::RenderObject(size_t id)
	: m_id(id), m_name{},
	m_modelID{}, m_meshIndex{},
	m_meshID{}, m_textureID{}, m_localTM(GOE::Matrix4x4::Identity()), m_isVisible(false), m_isAnimated(false)
{}


RenderObject::~RenderObject() = default;

