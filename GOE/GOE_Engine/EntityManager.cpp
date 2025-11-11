#include "Engine_pch.h"
#include "EntityManager.h"
#include "Entity.h"

EntityManager::EntityManager() = default;
EntityManager::~EntityManager() = default;

Entity* EntityManager::CreateEntity(std::string name)
{
	// 1. 새로운 엔티티를 생성하고 소유권을 unique_ptr로 관리합니다.
	EntityID newID = GOE::FileManager::GetHash(name);
	auto pNewEntity = std::make_unique<Entity>(newID, name);

	// 2. 조회용 맵에 ID와 size를 추가합니다.
	m_entityMap[newID] = m_entities.size();

	// 3. 소유권을 가진 포인터를 벡터에 추가합니다.
	m_entities.push_back(std::move(pNewEntity));

	return m_entities.back().get();
}

Entity* EntityManager::FindEntity(EntityID id)
{
	auto it = m_entityMap.find(id);
	if (it != m_entityMap.end())
	{
		return m_entities[it->second].get(); 
	}
	return nullptr;
}
