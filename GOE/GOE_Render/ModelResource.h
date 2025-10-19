#pragma once
#include <DirectXMath.h>
#include <windows.h>
#include <wrl.h>
#include <string>
#include <memory>
#include <vector>

class MeshResource;

/// <summary>
/// 렌더러에서 갖고 있을 모델정보 클래스
/// 
/// 어심프 구조에서 메쉬의 상위구조라 만들었지만
/// 현시점에선 필요없는 자료형
/// 
/// </summary>
class ModelResource
{
public:
	ModelResource(std::string name, size_t id)
		:m_name(name), m_id(id)	{}

	~ModelResource();

public:
	inline const std::string& GetName() const { return m_name; }
	inline size_t GetID() const { return m_id; }
	inline bool IsVisible() const { return isVisible; }
	
	inline const std::vector<std::unique_ptr<MeshResource>>& GetMeshResources() const { return m_meshResourceMap; }
	inline MeshResource* GetMeshResource(size_t id) { return m_meshResourceMap[id].get(); }
	inline MeshResource* GetMeshResourceBack() { return m_meshResourceMap.back().get(); }

public:
	inline void SetVisible(bool visible) { isVisible = visible; }
	
public:	
	inline void AddMeshResource(std::string name, size_t id) { m_meshResourceMap.push_back(std::make_unique<MeshResource>(name, id)); }

private:
	const std::string m_name = ""; // 모델 이름
	const size_t m_id = 0; // 모델 ID
	bool isVisible = false; // 모델이 보이는지 여부

	std::vector<std::unique_ptr<MeshResource>> m_meshResourceMap; // 메쉬 리소스들
};