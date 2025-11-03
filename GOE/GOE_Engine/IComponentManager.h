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
		m_ownerMap[ownerID] = m_components.size();
		m_componentMap[id] = m_components.size();
		m_components.emplace_back(T(ownerID, id));
	}

	inline std::vector<T>& GetComponents() { return m_components; }
	inline const std::vector<T>& GetComponents() const { return m_components; }

	inline T* GetCurrentComponent() { return &m_components.back(); }
	inline const T* GetComponent(ComponentID componentID) const
	{
		auto it = m_componentMap.find(componentID);
		if (it != m_componentMap.end())
		{
			return &m_components[m_componentMap[componentID]];
		}
		return nullptr;
	}

	inline T* GetComponent(ComponentID componentID)
	{
		auto it = m_componentMap.find(componentID);
		if (it != m_componentMap.end())
		{
			return &m_components[m_componentMap[componentID]];
		}
		return nullptr;
	}

	inline std::unordered_map<ComponentID, ComponentIndex>& GetComponentMap() { return m_componentMap; }
	inline const std::unordered_map<ComponentID, ComponentIndex>& GetComponentMap() const { return m_componentMap; }

	// OwnerID로 컴포넌트를 찾는 함수들을 추가합니다.
	inline T* GetComponentByOwner(OwnerID ownerID)
	{
		auto it = m_ownerMap.find(ownerID);
		if (it != m_ownerMap.end())
		{
			return &m_components[m_ownerMap[ownerID]];
		}
		return nullptr;
	}
	inline const T* GetComponentByOwner(OwnerID ownerID)  const
	{
		auto it = m_ownerMap.find(ownerID);
		if (it != m_ownerMap.end())
		{
			return &m_components[m_ownerMap[ownerID]];
		}
		return nullptr;
	}

protected:
	std::vector<T> m_components;
	std::unordered_map<ComponentID, ComponentIndex> m_componentMap;
	std::unordered_map<OwnerID, ComponentIndex> m_ownerMap; // OwnerID -> Index 맵 추가
};
