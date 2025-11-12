#include "Editor_pch.h"
#include "EditorCore.h"
#include "DebugManager.h"
DebugManager::DebugManager()
{
}

DebugManager::~DebugManager()
{
}

void DebugManager::OnDebugUpdate(double dTime)
{
	while(!m_debugQueue.empty())
	{
		m_debugQueue.front()();
		m_debugQueue.pop();
	}
}
void DebugManager::OnRender()
{
	
}
