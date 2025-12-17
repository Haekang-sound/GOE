#pragma once
using EntityID = size_t;
using ComponentID = size_t;


class Entity
{
private:
	EntityID m_entityID = 0;
	EntityID m_parentID = 0;
	std::string m_name;
	std::vector<EntityID> m_children;
	std::vector<ComponentID> m_componentID;

public:
	Entity(EntityID id, std::string name);
	~Entity();

public:
	bool HasParent() { return m_parentID != 0; }
	bool HasChildren() { return m_children.size() != 0; }

public:
	EntityID GetEntitiyID() { return m_entityID; }
	std::string GetName() { return m_name; }
	EntityID GetParentID() { return m_parentID; }
	std::vector<EntityID>& GetChildern() { return m_children; }

public:
	void SetName(std::string name) { m_name = name; }
	void SetParent(EntityID id) { m_parentID = id; }
	void SetChild(EntityID id) { m_children.push_back(id); }
};

