#pragma once
class MeshResource
{
public:
	MeshResource(std::string name, size_t id)
		: m_name(name), m_id(id)	{}

	~MeshResource();
public:
	inline const std::string& GetName() const { return m_name; }
	inline size_t GetID() const { return m_id; }
	
	inline const UINT64& GetVBSize() const { return m_VBSize; }
	inline ID3D12Resource* GetVBUpload() const { return m_vertexBufferUpload.Get(); }
	inline ID3D12Resource* GetVBDefault() const { return m_vertexBufferDefault.Get(); }
	inline const D3D12_VERTEX_BUFFER_VIEW& GetVBView() const { return m_vertexBufferView; }

	inline const UINT64& GetIndexCount() const { return m_IndexCount; }
	inline const UINT64& GetIBSize() const { return m_IBSize; }
	inline ID3D12Resource* GetIBUpload() const { return m_indexBufferUpload.Get(); }
	inline ID3D12Resource* GetIBDefault() const { return m_indexBufferDefault.Get(); }
	inline const D3D12_INDEX_BUFFER_VIEW& GetIBView() const { return m_indexBufferView; }

	inline ID3D12Resource* GetCB() const { return m_constantBuffer.Get(); }
	inline ID3D12DescriptorHeap* GetCBVHeap() const { return m_CBVHeap.Get(); }
	inline const D3D12_CPU_DESCRIPTOR_HANDLE& GetCBVHandle() const { return m_CBVHandle; }
	

public:
	inline void SetVBSize(UINT64 size) { m_VBSize = size; }
	inline void SetVBUpload(ComPtr<ID3D12Resource>&& uploadBuffer) { m_vertexBufferUpload = std::move(uploadBuffer); }
	inline void SetVBDefault(ComPtr<ID3D12Resource>&& defaultBuffer) { m_vertexBufferDefault = std::move(defaultBuffer); }
	inline void SetVBView(const D3D12_VERTEX_BUFFER_VIEW& view) { m_vertexBufferView = view; }

	inline void SetIBSize(UINT64 size) { m_IBSize = size; }
	inline void Setm_IndexCount(UINT64 num) { m_IndexCount = num; }
	inline void SetIBUpload(ComPtr<ID3D12Resource>&& uploadBuffer) { m_indexBufferUpload = std::move(uploadBuffer); }
	inline void SetIBDefault(ComPtr<ID3D12Resource>&& defaultBuffer) { m_indexBufferDefault = std::move(defaultBuffer); }
	inline void SetIBView(const D3D12_INDEX_BUFFER_VIEW& view) { m_indexBufferView = view; }

	inline void SetCB(ComPtr<ID3D12Resource>&& uploadBuffer) { m_constantBuffer = std::move(uploadBuffer); }
	inline void SetCBVHeap(ComPtr<ID3D12DescriptorHeap>&& heap) { m_CBVHeap = std::move(heap); }
	inline void SetCBVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE&& handle) { m_CBVHandle = std::move(handle); }

private:
	std::string m_name; // 모델 이름
	const size_t m_id = 0; // 모델 ID

	// 모델을 그릴때 필요한 리소스들을 갖고있다.
private:
	UINT64 m_VBSize = 0;
	ComPtr<ID3D12Resource> m_vertexBufferUpload = nullptr;
	ComPtr<ID3D12Resource> m_vertexBufferDefault = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

	UINT64 m_IBSize = 0;
	UINT64 m_IndexCount = 0;
	ComPtr<ID3D12Resource> m_indexBufferUpload = nullptr;
	ComPtr<ID3D12Resource> m_indexBufferDefault = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};

	// CB
	ComPtr<ID3D12Resource> m_constantBuffer = {};
	ComPtr<ID3D12DescriptorHeap> m_CBVHeap = {};
	D3D12_CPU_DESCRIPTOR_HANDLE m_CBVHandle = {};

public:
	void operator=(const Mesh& core_Mesh);
};

