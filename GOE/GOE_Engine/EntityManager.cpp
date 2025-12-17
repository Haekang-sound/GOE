#include "Engine_pch.h"
#include "EntityManager.h"
#include "Entity.h"

/// <summary>
/// 이름으로 엔티티를 생성합니다.
/// </summary>
/// <param name="name">엔티티 이름</param>
/// <returns>생성된 Entity포인터</returns>
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

/// <summary>
/// ID로 엔티티를 찾는 함수
/// </summary>
/// <param name="id"></param>
/// <returns></returns>
Entity* EntityManager::FindEntity(EntityID id)
{
	auto it = m_entityMap.find(id);
	if (it != m_entityMap.end())
	{
		return m_entities[it->second].get(); 
	}
	return nullptr;
}

/// <summary>
/// 부모자식 설정함수
/// 
/// </summary>
/// <param name="_parent">부모</param>
/// <param name="_child">자식</param>
void EntityManager::SetParent(Entity* _parent, Entity* _child)
{
	_parent->SetChild(_child->GetEntitiyID());
	_child->SetParent(_parent->GetEntitiyID());
}

/// <summary>
/// 부모자식 설정함수
/// By ID
/// </summary>
/// <param name="_parent">부모id</param>
/// <param name="_child">자식id</param>
void EntityManager::SetParentByID(EntityID _parent, EntityID _child)
{
	FindEntity(_parent)->SetChild(_child);
	FindEntity(_child)->SetParent(_parent);
}
