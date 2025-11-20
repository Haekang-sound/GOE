#pragma once
/// <summary>
/// 메쉬정보를 랜더러에서 사용할 수 있는 형태로 가공한 클래스
/// 
/// </summary>
class MeshResource
{
public:
	MeshResource(std::string name, size_t id)
		: m_name(name), m_id(id)	{}

	~MeshResource();
public:
	inline const std::string& GetName() const { return m_name; }
	inline size_t GetID() const { return m_id; }
	inline size_t GetMeshIndex() const { return m_meshIndex; }
	inline size_t GetModelID() const { return m_modelID; }

	inline const UINT& GetVBSize() const { return m_VBSize; }
	inline ID3D12Resource* GetVBDefault() const { return m_vertexBufferDefault.Get(); }
	inline const D3D12_VERTEX_BUFFER_VIEW& GetVBView() const { return m_vertexBufferView; }

	inline const UINT& GetIndexCount() const { return m_IndexCount; }
	inline const UINT& GetIBSize() const { return m_IBSize; }
	inline ID3D12Resource* GetIBDefault() const { return m_indexBufferDefault.Get(); }
	inline const D3D12_INDEX_BUFFER_VIEW& GetIBView() const { return m_indexBufferView; }

	inline ID3D12Resource* GetCB() const { return m_constantBuffer.Get(); }
	//inline ID3D12DescriptorHeap* GetCBVHeap() const { return m_CBVHeap.Get(); }
	//inline const D3D12_CPU_DESCRIPTOR_HANDLE& GetCBVHandle() const { return m_CBVHandle; }

public:
	inline void SetMeshIndex(size_t index) { m_meshIndex = index; }
	inline void SetModelID(size_t modelID) { m_modelID = modelID; }

	inline void SetVBSize(UINT size) { m_VBSize = size; }
	inline void SetVBDefault(ComPtr<ID3D12Resource>&& defaultBuffer) { m_vertexBufferDefault = std::move(defaultBuffer); }
	inline void SetVBView(const D3D12_VERTEX_BUFFER_VIEW& view) { m_vertexBufferView = view; }

	inline void SetIBSize(UINT size) { m_IBSize = size; }
	inline void SetIndexCount(UINT num) { m_IndexCount = num; }
	inline void SetIBDefault(ComPtr<ID3D12Resource>&& defaultBuffer) { m_indexBufferDefault = std::move(defaultBuffer); }
	inline void SetIBView(const D3D12_INDEX_BUFFER_VIEW& view) { m_indexBufferView = view; }

	inline void SetCB(ComPtr<ID3D12Resource>&& uploadBuffer) { m_constantBuffer = std::move(uploadBuffer); }

private:
	std::string m_name; // 모델 이름
	const size_t m_id = 0; // 모델 ID
	size_t m_meshIndex = -1; // 메쉬 인덱스 (추가 필요시 사용)
	size_t m_modelID = 0; // 모델 ID (추가 필요시 사용)

	// 모델을 그릴때 필요한 리소스들을 갖고있다.
private:
	UINT m_VBSize = 0;
	ComPtr<ID3D12Resource> m_vertexBufferDefault = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

	UINT m_IBSize = 0;
	UINT m_IndexCount = 0;
	ComPtr<ID3D12Resource> m_indexBufferDefault = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};

	// boneoffset을 저장할 자료형
	UINT m_CBSize = 0;
	ComPtr<ID3D12Resource> m_constantBuffer = {};
	//ComPtr<ID3D12DescriptorHeap> m_CBVHeap = {};
	//D3D12_CPU_DESCRIPTOR_HANDLE m_CBVHandle = {};
};