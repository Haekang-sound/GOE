#include "DebugManager.h"

DebugManager::DebugManager()
{
}

DebugManager::~DebugManager()
{
}

void DebugManager::OnDebugUpdate()
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
