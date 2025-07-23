#pragma once
#include <DirectXMath.h>
#include <windows.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

/// <summary>
/// 큐브오브젝트를 구현하기 위한 클래스 
///
/// 2025.06.30 ohk
/// </summary>
class Cube
{
public:
	Cube();
	Cube(ComPtr<ID3D12Device> device, float aspectRatio);
	~Cube();

	void InitCube();
	void LoadCube();
	void OnUpdate();
	void OnRender(const ComPtr<ID3D12GraphicsCommandList>& commadList);

	void CreateVertexBuffer();
	void SetVertexBufferView();
	void CreateIndexBuffer();
	void CreateConstantBuffer();

	void CopyUploadHeapToDefault(const ComPtr<ID3D12GraphicsCommandList>& commadList);

public: 
	inline XMMATRIX GetLocalTransForm() { return XMLoadFloat4x4(&m_local); }

public:
	// device
	ComPtr<ID3D12Device> m_device = nullptr;
	float m_aspectRatio = 0.f;
	// 월드 매트릭스
	XMFLOAT4X4 m_local = {};
	float m_moveSpeed = 0.25f;
	XMFLOAT3 m_position = { 0,0,0 };
	XMFLOAT3 m_rotation = { 0,0,0 };
	XMFLOAT3 m_scale = { 1,1,1 };

	float m_angle = 0.f;

	/// <summary>
	/// 이 구간은 리소스를 갖는다.
	/// </summary>
public:
	// VS
	Vertex m_triangleVertices[24] = {};
	UINT m_vertexBufferSize = 0;
	ComPtr<ID3D12Resource> m_vertexBufferUpload = nullptr;
	ComPtr<ID3D12Resource> m_vertexBufferDefault = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};

	// CB
	ComPtr<ID3D12DescriptorHeap> m_cbvHeap = {};
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_cbvDesc = {};
	D3D12_CPU_DESCRIPTOR_HANDLE m_cbvHandle = {};
	ComPtr<ID3D12Resource> m_constantBuffer = {};

	// 인덱스
	UINT m_indexBufferSize = 0;
	ComPtr<ID3D12Resource> m_indexBufferUpload = nullptr;
	ComPtr<ID3D12Resource> m_indexBufferDefault = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView = {};

};

