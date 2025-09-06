#include "Renderer_pch.h"
#include "RenderObject.h"

RenderObject::RenderObject(RenderObjectData& data)
		: m_id(data.id), m_name(data.name),
		m_modelID(data.modelID), m_meshIndex(data.meshIndex),
		m_meshID(data.meshID), m_textureID(data.textureID), m_localTM(data.localTM){}

RenderObject::~RenderObject() = default; // 소멸자 구현

