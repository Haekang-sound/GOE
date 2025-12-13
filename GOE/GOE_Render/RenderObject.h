#pragma once
#include <string>
namespace Graphics
{
	struct Matrix4x4;

}


/// <summary>
/// 랜더링에 필요한 
/// 오브젝트의 고유리소스를 담고있는 
/// 랜더오브젝트
/// 
/// </summary>
class RenderObject
{
public:
	RenderObject(size_t id);
	~RenderObject();

public:
	inline const std::string& GetName() const { return m_name; }
	inline size_t GetID() const { return m_id; }
	inline size_t GetMeshID() const { return m_meshID; }
	inline size_t GetMeshIndex() const { return m_meshIndex; }
	inline size_t GetModelID() const { return m_modelID; }
	inline size_t GetTextureID() const { return m_textureID; }
	inline Graphics::Matrix4x4& GetLocalTM() { return m_localTM; }
	inline Graphics::Matrix4x4& GetWorldTM() { return m_worldTM; }
	inline bool IsVisible() const { return m_isVisible; }
	inline bool IsAnimated() const { return m_isAnimated; }
	inline Graphics::Matrix4x4& GetBoneTM(int i) { return m_boneTM[i]; }
	inline Graphics::Matrix4x4* GetBoneTMBegin() { return m_boneTM; }
	inline size_t GetEntityID() { return m_entityID; }

public:
	inline void SetVisible(bool visible) { m_isVisible = visible; }
	inline void SetAnimated(bool animated) { m_isAnimated = animated; }

	inline void SetMeshID(size_t meshID) { m_meshID = meshID; }
	inline void SetMeshIndex(size_t meshIndex) { m_meshIndex = meshIndex; }
	inline void SetModelID(size_t modelID) { m_modelID = modelID; }
	inline void SetTextureID(size_t textureID) { m_textureID = textureID; }
	inline void SetLocalTM(const GOE::Matrix4x4& localTM) { m_localTM = localTM; }
	inline void SetWorldTM(const GOE::Matrix4x4& worldTM) { m_worldTM = worldTM; }
	inline void SetBoneTM(size_t idx, GOE::Matrix4x4 matrix)
	{
		m_boneTM[idx] = matrix;
	}
	inline void SetEntityID(size_t id) { m_entityID = id; }

private:
	const size_t m_id = 0; // 오브젝트 ID
	size_t m_meshID = 0; // 메쉬 ID
	size_t m_meshIndex = 0; // 메쉬 index 
	size_t m_textureID = 0; // 텍스처 ID 
	size_t m_modelID = 0; // 모델 ID 
	size_t m_entityID = 0;
	
	Graphics::Matrix4x4 m_localTM = GOE::Matrix4x4::Identity();
	Graphics::Matrix4x4 m_worldTM = GOE::Matrix4x4::Identity();
	Graphics::Matrix4x4 m_boneTM[128];

	std::string m_name = ""; // 오브젝트 이름
	bool m_isVisible = false; // 오브젝트가 보이는지 여부
	bool m_isAnimated = false; // 애니메이션 적용 여부
};

