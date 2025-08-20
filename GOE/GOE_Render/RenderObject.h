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
	inline Graphics::Matrix4x4& GetLocalTM() { return m_localTM; }

	inline bool IsVisible() const { return isVisible; }
	inline const std::string& GetName() const { return m_name; }

	inline ID3D12Resource* GetCB() const { return m_constantBuffer.Get(); }
	inline ID3D12DescriptorHeap* GetCBVHeap() const { return m_CBVHeap.Get(); }
	inline const D3D12_CPU_DESCRIPTOR_HANDLE& GetCBVHandle() const { return m_CBVHandle; }

public:
	inline void SetVisible(bool visible) { isVisible = visible; }
	inline void SetMeshID(size_t meshID) { m_meshID = meshID; }
	inline void SetMeshIndex(size_t meshIndex) { m_meshIndex = meshIndex; }
	inline void SetModelID(size_t modelID) { m_modelID = modelID; }

	inline void SetCB(ComPtr<ID3D12Resource>&& uploadBuffer) { m_constantBuffer = std::move(uploadBuffer); }
	inline void SetCBVHeap(ComPtr<ID3D12DescriptorHeap>&& heap) { m_CBVHeap = std::move(heap); }
	inline void SetCBVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE&& handle) { m_CBVHandle = handle; }

	inline void SetLocalTM(const GOE::Matrix4x4& localTM) { m_localTM = localTM; }
private:
	const size_t m_id = 0; // 오브젝트 ID
	size_t m_meshID = 0; // 메쉬 ID
	size_t m_meshIndex = 0; // 메쉬 index (추가 필요시 사용)
	size_t m_modelID = 0; // 모델 ID (추가 필요시 사용)
	Graphics::Matrix4x4 m_localTM;

	std::string m_name = ""; // 오브젝트 이름
	bool isVisible = true; // 오브젝트가 보이는지 여부

private:
	// CB
	ComPtr<ID3D12Resource> m_constantBuffer = {};
	ComPtr<ID3D12DescriptorHeap> m_CBVHeap = {};
	D3D12_CPU_DESCRIPTOR_HANDLE m_CBVHandle = {};

};

