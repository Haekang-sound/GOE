#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <windows.h>
#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <stdexcept>
#include <comdef.h>

#include "ID3DRenderer.h"
#include "GOETypes.h"


using Microsoft::WRL::ComPtr;
using namespace Microsoft::WRL;
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

	void CreateVertexBuffer();
	void SetVertexBufferView();
	void CreateIndexBuffer();
	void CreateConstantBuffer();

	void CopyUploadHeapToDefault(const ComPtr<ID3D12GraphicsCommandList>& commadList);

	void SetDrawCube(const ComPtr<ID3D12GraphicsCommandList>& commadList);

public: 
	inline XMMATRIX GetLocalTransForm() { return XMLoadFloat4x4(&m_local); }

public:
	// device
	ComPtr<ID3D12Device> m_device = nullptr;
	float m_aspectRatio = 0.f;
	// 월드 매트릭스
	XMFLOAT4X4 m_local = {};

	XMFLOAT3 m_position;
	XMFLOAT3 m_rotation;
	XMFLOAT3 m_scale;

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


public:
	XMFLOAT4 m_faceColors[6] =
	{
		{1, 0, 0, 1}, // 앞면
		{0, 1, 0, 1}, // 뒷면
		{0, 0, 1, 1}, // 오른쪽
		{1, 1, 0, 1}, // 왼쪽
		{0, 1, 1, 1}, // 윗면
		{1, 0, 1, 1}  // 아랫면
	};

	Vertex m_vertexArray[24] =
	{
		// 앞면 (z = +0.25f)
		{ {-0.25f,  0.25f ,  0.25f},	{ m_faceColors[0].x, m_faceColors[0].y, m_faceColors[0].z, m_faceColors[0].w } }, // 0
		{ {0.25f,  0.25f ,  0.25f},		{ m_faceColors[0].x, m_faceColors[0].y, m_faceColors[0].z, m_faceColors[0].w } }, // 1
		{ {0.25f, -0.25f ,  0.25f},		{ m_faceColors[0].x, m_faceColors[0].y, m_faceColors[0].z, m_faceColors[0].w } }, // 2
		{ {-0.25f, -0.25f ,  0.25f},	{ m_faceColors[0].x, m_faceColors[0].y, m_faceColors[0].z, m_faceColors[0].w } }, // 3

		// 뒷면 (z = -0.25f)
		{ {-0.25f,  0.25f , -0.25f},	{ m_faceColors[1].x, m_faceColors[1].y, m_faceColors[1].z, m_faceColors[1].w } }, // 4
		{ {0.25f,  0.25f , -0.25f},		{ m_faceColors[1].x, m_faceColors[1].y, m_faceColors[1].z, m_faceColors[1].w } }, // 5
		{ {0.25f, -0.25f , -0.25f},		{ m_faceColors[1].x, m_faceColors[1].y, m_faceColors[1].z, m_faceColors[1].w } }, // 6
		{ {-0.25f, -0.25f , -0.25f},	{ m_faceColors[1].x, m_faceColors[1].y, m_faceColors[1].z, m_faceColors[1].w } }, // 7

		// 오른쪽면 (x = +0.25f)
		{ {0.25f,  0.25f ,  0.25f},		{ m_faceColors[2].x, m_faceColors[2].y, m_faceColors[2].z, m_faceColors[2].w} }, // 8
		{ {0.25f,  0.25f , -0.25f},		{ m_faceColors[2].x, m_faceColors[2].y, m_faceColors[2].z, m_faceColors[2].w} }, // 9
		{ {0.25f, -0.25f , -0.25f},		{ m_faceColors[2].x, m_faceColors[2].y, m_faceColors[2].z, m_faceColors[2].w} }, // 10
		{ {0.25f, -0.25f ,  0.25f},		{ m_faceColors[2].x, m_faceColors[2].y, m_faceColors[2].z, m_faceColors[2].w} }, // 11

		// 왼쪽면 (x = -0.25f)
		{ {-0.25f,  0.25f , -0.25f},	{ m_faceColors[3].x, m_faceColors[3].y, m_faceColors[3].z, m_faceColors[3].w } }, // 12
		{ {-0.25f,  0.25f ,  0.25f},	{ m_faceColors[3].x, m_faceColors[3].y, m_faceColors[3].z, m_faceColors[3].w } }, // 13
		{ {-0.25f, -0.25f ,  0.25f},	{ m_faceColors[3].x, m_faceColors[3].y, m_faceColors[3].z, m_faceColors[3].w } }, // 14
		{ {-0.25f, -0.25f , -0.25f},	{ m_faceColors[3].x, m_faceColors[3].y, m_faceColors[3].z, m_faceColors[3].w } }, // 15

		// 윗면 (y = +0.25f )
		{ {-0.25f,  0.25f , -0.25f},	{ m_faceColors[4].x, m_faceColors[4].y, m_faceColors[4].z, m_faceColors[4].w } }, // 16
		{ {0.25f,  0.25f , -0.25f},		{ m_faceColors[4].x, m_faceColors[4].y, m_faceColors[4].z, m_faceColors[4].w } }, // 17
		{ {0.25f,  0.25f ,  0.25f},		{ m_faceColors[4].x, m_faceColors[4].y, m_faceColors[4].z, m_faceColors[4].w } }, // 18
		{ {-0.25f,  0.25f ,  0.25f},	{ m_faceColors[4].x, m_faceColors[4].y, m_faceColors[4].z, m_faceColors[4].w } }, // 19

		// 아랫면 (y = -0.25f )
		{ {-0.25f, -0.25f ,  0.25f},	{ m_faceColors[5].x, m_faceColors[5].y, m_faceColors[5].z, m_faceColors[5].w } }, // 20
		{ {0.25f, -0.25f ,  0.25f},		{ m_faceColors[5].x, m_faceColors[5].y, m_faceColors[5].z, m_faceColors[5].w } }, // 21
		{ {0.25f, -0.25f , -0.25f},		{ m_faceColors[5].x, m_faceColors[5].y, m_faceColors[5].z, m_faceColors[5].w } }, // 22
		{ {-0.25f, -0.25f , -0.25f},	{ m_faceColors[5].x, m_faceColors[5].y, m_faceColors[5].z, m_faceColors[5].w } }  // 23
	};
};

