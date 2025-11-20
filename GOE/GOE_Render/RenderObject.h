#pragma once
#include <string>
namespace Graphics
{
	struct Matrix4x4;

}

struct RenderObjectData;

/// <summary>
/// 랜더링에 필요한 
/// 오브젝트의 고유리소스를 담고있는 
/// 랜더오브젝트
/// 
/// </summary>
class RenderObject
{
public:
	RenderObject(RenderObjectData& data);
	~RenderObject();

public:
	inline size_t GetID() const { return m_id; }
	inline size_t GetMeshID() const { return m_meshID; }
	inline size_t GetMeshIndex() const { return m_meshIndex; }
	inline size_t GetModelID() const { return m_modelID; }
	inline size_t GetTextureID() const { return m_textureID; }
	inline Graphics::Matrix4x4& GetLocalTM() { return m_localTM; }
	inline Graphics::Matrix4x4& GetWorldTM() { return m_worldTM; }
	inline bool IsVisible() const { return m_isVisible; }
	inline bool IsAnimated() const { return m_isAnimated; }

	inline const std::string& GetName() const { return m_name; }

	inline ID3D12Resource* GetCB() const { return m_constantBuffer.Get(); }
	inline ID3D12Resource* GetCB() { return m_constantBuffer.Get(); }
	inline ID3D12DescriptorHeap* GetCBVHeap() const { return m_CBVHeap.Get(); }
	inline const D3D12_CPU_DESCRIPTOR_HANDLE& GetCBVHandle() const { return m_CBVHandle; }

	inline ID3D12Resource* GetCBBoneMatrix() const { return m_boneMatrixBuffer.Get(); }
	inline ID3D12DescriptorHeap* GetCBVoneMatrixHeap() const { return m_boneMatrixHeap.Get(); }
	inline const D3D12_CPU_DESCRIPTOR_HANDLE& SetCBVoneMatrixHandle() const { return m_boneMatrixHandle; }

	inline Graphics::Matrix4x4& GetBoneTM(int i) { return m_boneTM[i]; }
	inline Graphics::Matrix4x4* GetBoneTMBegin() { return m_boneTM; }

public:
	inline void SetVisible(bool visible) { m_isVisible = visible; }
	inline void SetAnimated(bool animated) { m_isAnimated = animated; }

	inline void SetMeshID(size_t meshID) { m_meshID = meshID; }
	inline void SetMeshIndex(size_t meshIndex) { m_meshIndex = meshIndex; }
	inline void SetModelID(size_t modelID) { m_modelID = modelID; }
	inline void SetTextureID(size_t textureID) { m_textureID = textureID; }

	inline void SetCB(ComPtr<ID3D12Resource>&& uploadBuffer) { m_constantBuffer = std::move(uploadBuffer); }
	inline void SetCBVHeap(ComPtr<ID3D12DescriptorHeap>&& heap) { m_CBVHeap = std::move(heap); }
	inline void SetCBVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE&& handle) { m_CBVHandle = handle; }

	inline void SetCBBoneMatrix(ComPtr<ID3D12Resource>&& uploadBuffer) { m_boneMatrixBuffer = std::move(uploadBuffer); }
	inline void SetCBVoneMatrixHeap(ComPtr<ID3D12DescriptorHeap>&& heap) { m_boneMatrixHeap = std::move(heap); }
	inline void SetCBVoneMatrixHandle(const D3D12_CPU_DESCRIPTOR_HANDLE&& handle) { m_boneMatrixHandle = handle; }


	inline void SetLocalTM(const GOE::Matrix4x4& localTM) { m_localTM = localTM; }
	inline void SetWorldTM(const GOE::Matrix4x4& worldTM) { m_worldTM = worldTM; }
	inline void SetBoneTM(size_t idx, GOE::Matrix4x4 matrix)
	{
		m_boneTM[idx] = matrix;// .Transpose();
	}
private:
	const size_t m_id = 0; // 오브젝트 ID
	size_t m_meshID = 0; // 메쉬 ID
	size_t m_meshIndex = 0; // 메쉬 index 
	size_t m_textureID = 0; // 텍스처 ID 
	size_t m_modelID = 0; // 모델 ID 
	
	Graphics::Matrix4x4 m_localTM = GOE::Matrix4x4::Identity();
	Graphics::Matrix4x4 m_worldTM = GOE::Matrix4x4::Identity();
	Graphics::Matrix4x4 m_boneTM[128];

	std::string m_name = ""; // 오브젝트 이름
	bool m_isVisible = false; // 오브젝트가 보이는지 여부
	bool m_isAnimated = false; // 애니메이션 적용 여부

private:
	// CB
	ComPtr<ID3D12Resource> m_constantBuffer = {};
	ComPtr<ID3D12DescriptorHeap> m_CBVHeap = {};
	D3D12_CPU_DESCRIPTOR_HANDLE m_CBVHandle = {};

	ComPtr<ID3D12Resource> m_boneMatrixBuffer = {};
	ComPtr<ID3D12DescriptorHeap> m_boneMatrixHeap = {};
	D3D12_CPU_DESCRIPTOR_HANDLE m_boneMatrixHandle = {};

};

