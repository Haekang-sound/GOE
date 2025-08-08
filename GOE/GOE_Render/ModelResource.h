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
/// </summary>
class ModelResource
{
public:
	ModelResource(std::string name, size_t id)
		:m_name(name), m_id(id)
	{
	}

	~ModelResource();

public:
	inline const std::string& GetName() const { return m_name; }
	inline size_t GetID() const { return m_id; }
	
	inline const std::vector<std::unique_ptr<MeshResource>>& GetMeshResources() const { return m_meshResources; }
	inline MeshResource* GetMeshResource(size_t id) { return m_meshResources[id].get(); }
	inline MeshResource* GetMeshResourceBack() { return m_meshResources.back().get(); }

public:	
	inline void AddMeshResource(std::string name, size_t id) { m_meshResources.push_back(std::make_unique<MeshResource>(name, id)); }

private:
	const std::string m_name = ""; // 모델 이름
	const size_t m_id = 0; // 모델 ID

	std::vector<std::unique_ptr<MeshResource>> m_meshResources; // 메쉬 리소스들
};