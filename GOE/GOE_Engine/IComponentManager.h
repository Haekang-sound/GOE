#pragma once
#include<vector>
#include<memory>
using ComponentID = size_t;
using ComponentIndex = size_t;
using OwnerID = size_t;

class IComponentManager
{
public:
	IComponentManager() = default;
	virtual ~IComponentManager() {}
};

template<typename T>
class ComponentManager : public IComponentManager
{
public:
	inline void AddComponent(OwnerID ownerID, ComponentID id)
	{
		m_componentMap[id] = m_components.size();
		m_components.emplace_back(T(ownerID, id));
	}

	inline std::vector<T>& GetComponents()  { return m_components; }
	inline const std::vector<T>& GetComponents() const { return m_components; }
	inline T& GetCurrentComponent() { return m_components.back(); }
	inline std::unordered_map<ComponentID, ComponentIndex>& GetComponentMap() { return    m_componentMap; }
	inline const std::unordered_map<ComponentID, ComponentIndex>& GetComponentMap() const { return    m_componentMap; }

protected:
	std::vector<T> m_components;
	std::unordered_map<ComponentID, ComponentIndex> m_componentMap;
};
