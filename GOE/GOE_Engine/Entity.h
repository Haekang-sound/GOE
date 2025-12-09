#pragma once
using EntityID = size_t;
using ComponentID = size_t;


class Entity
{
public:
	Entity(EntityID id, std::string name);
	~Entity();

public:
	EntityID GetEntitiyID() { return m_entityID; }
	std::string GetName() { return m_name; }

public:
	void SetName(std::string name) { m_name = name; }

private:
	EntityID m_entityID = 0;
	std::string m_name;

	std::vector<ComponentID> m_componentID;
};

