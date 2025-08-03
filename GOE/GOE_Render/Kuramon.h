#pragma once
#include <DirectXMath.h>
#include <windows.h>
#include <wrl.h>

namespace GOE
{
	struct MeshData;
}

using Microsoft::WRL::ComPtr;
using namespace DirectX;

class Kuramon
{
public:
	Kuramon();
	Kuramon(ComPtr<ID3D12Device> device, float aspectRatio);
	~Kuramon();

	// 복사생성자
	Kuramon(const Kuramon&) = delete;
	// 복사 대입 연산자
	Kuramon& operator=(const Kuramon&) = delete;
	// 이동생성자
	Kuramon(Kuramon&&) = delete;
	// 이동 대입 연산자
	Kuramon& operator=(Kuramon&&) = delete;


	void InitKuramon();
	void LoadKuramon();
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
	void DirextXVertexToKuramonVertex();

public:
	// device
	ComPtr<ID3D12Device> m_device = nullptr;
	float m_aspectRatio = 0.f;
	// 월드 매트릭스
	XMFLOAT4X4 m_local = {};

	XMFLOAT3 m_position = { 0,0,0 };
	XMFLOAT3 m_rotation = { 0,0,0 };
	XMFLOAT3 m_scale = { 1,1,1 };

	float m_moveSpeed = 0.25f; // 이동 속도
	float m_angle = 0.f; // 회전 각도
	
public:
	/// <summary>
	/// 오류를 잡기위해 임시로 만들어둔 변수2개
	/// 사용하고 있지만 변경되어야함
	/// 
	/// </summary>
	std::vector<Graphics::Vertex> vertexArray;
	std::vector<UINT32> indexArray;
	GOE::MeshData* m_kuramonMeshData = nullptr;

	std::vector<Graphics::Vertex> m_kuramonTriangleVertices; // 얘만 어떻게 하면 될지도? 
	UINT m_vertexBufferSize = 0;
	ComPtr<ID3D12Resource> m_kuramonVertexBufferUpload = nullptr;
	ComPtr<ID3D12Resource> m_kuramonVertexBufferDefault = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_kuramonVertexBufferView = {};

	// CB
	ComPtr<ID3D12DescriptorHeap> m_kuramonCBVHeap = {};
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_kuramonCBVDesc = {};
	D3D12_CPU_DESCRIPTOR_HANDLE m_kuramonCBVHandle = {};
	ComPtr<ID3D12Resource> m_kuramonConstantBuffer = {};

	// 인덱스
	UINT m_kuramonIndexBufferSize = 0;
	ComPtr<ID3D12Resource> m_kuramonIndexBufferUpload = nullptr;
	ComPtr<ID3D12Resource> m_kuramonIndexBufferDefault = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_kuramonIndexBufferView = {};

};

