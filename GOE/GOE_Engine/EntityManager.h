#pragma once
using EntityID = size_t;
using EntityIndex = size_t;

#include <memory>
#include <vector>
#include <unordered_map>

class Entity;
/// <summary>
/// 엔티티 생성관리의 
/// 책임을 맡는 엔티티매니저
/// 
/// </summary>
class EntityManager
{
private:
	std::vector<std::unique_ptr<Entity>> m_entities;
	std::unordered_map<EntityID, EntityIndex> m_entityMap;

public:
	Entity* CreateEntity(std::string name);
	Entity* FindEntity(EntityID id);
	void SetParent(Entity* _parent, Entity* _child);
	void SetParentByID(EntityID _parent, EntityID _child);

	/// <summary>
	///  엔티티를 파괴하는함수 (구현에정)
	/// </summary>
	/// <param name="id"></param>
	void DestroyEntity(EntityID id) {};

public:
	// 모든 엔티티에 대한 빠른 순회 접근
	const std::vector<std::unique_ptr<Entity>>& GetAllEntities() const { return m_entities; }
	std::vector<std::unique_ptr<Entity>>& GetAllEntities() { return m_entities; }
	std::unordered_map<EntityID, EntityIndex>& GetEntityMap() { return m_entityMap; }
	const std::unordered_map<EntityID, EntityIndex>& GetEntityMap() const { return m_entityMap; }
};




