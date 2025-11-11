#pragma once
#include "IComponent.h"
#include <string>

class MeshRenderer : public IComponent
{
public: 
	MeshRenderer(size_t ownerID, size_t id);
	~MeshRenderer();

public:
	size_t GetMeshID() const { return m_meshID; }
	size_t GetMeshIndex() const { return m_meshIndex; }
	size_t GetModelID() const { return m_modelID; }
	bool IsVisible() const { return m_isVisible; }
	const std::string& GetName() const { return m_name; }

public: 	
	void SetMeshID(size_t meshID) { m_meshID = meshID; }
	void SetMeshIndex(size_t meshIndex) { m_meshIndex = meshIndex; }
	void SetModelID(size_t modelID) { m_modelID = modelID; }
	void SetName(const std::string name) { m_name = name; }
	void SetVisible(bool isVisible) { m_isVisible = isVisible; }
	
protected:
	/// 이걸가져가서 
	size_t m_meshID = 0; // 메쉬 ID
	size_t m_meshIndex = 0; // 메쉬 index (추가 필요시 사용)
	size_t m_modelID = 0; // 모델 ID (추가 필요시 사용)
	bool m_isVisible = true; // 렌더링 여부

	std::string m_name = ""; // 오브젝트 이름
};

