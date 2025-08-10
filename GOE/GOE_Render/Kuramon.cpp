#include "Renderer_pch.h"
#include "Kuramon.h"

#include "../GOE_Editor/DebugManager.h"
#include "../Imgui/imgui.h"



Kuramon::Kuramon() {}

Kuramon::Kuramon(ComPtr<ID3D12Device> device, float aspectRatio)
	: m_device(device), m_aspectRatio(aspectRatio)
{
	m_kuramonMeshData = new GOE::MeshData();
}

Kuramon::~Kuramon()
{
	delete m_kuramonMeshData;
}

void Kuramon::InitKuramon()
{
	m_local._11 = 1;
	m_local._22 = 1;
	m_local._33 = 1;
	m_local._44 = 1;
}

void Kuramon::OnUpdate()
{
	// 기저벡터
	XMVECTOR right = XMVector3Normalize(XMVectorSet(m_local._11, m_local._21, m_local._31, 0.0f));
	XMVECTOR up = XMVector3Normalize(XMVectorSet(m_local._12, m_local._22, m_local._32, 0.0f));
	XMVECTOR forward = XMVector3Normalize(XMVectorSet(m_local._13, m_local._23, m_local._33, 0.0f));


	// 카메라 이동
	XMVECTOR pos = XMLoadFloat3(&m_position);
	if (GetAsyncKeyState(VK_UP) & 0x8000)	pos += forward * m_moveSpeed;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)	pos -= right * m_moveSpeed;
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)	pos -= forward * m_moveSpeed;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)	pos += right * m_moveSpeed;


	// (1) 각 변환을 XMMATRIX로 생성
	XMMATRIX S = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	XMMATRIX R = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	XMMATRIX T = XMMatrixTranslationFromVector(pos);

	// (2) 합성
	XMMATRIX world = S * R * T;

	XMStoreFloat4x4(&m_local, world);
	XMStoreFloat3(&m_position, pos);
	DebugManager::GetInstance().PushDebugData(
		[this]()
		{
			static float f = 0.0f;
			static int counter = 0;
			ImGui::Begin("쿠라몬!");
			ImGui::Text("조작법 : ←↑↓→");
			ImGui::Text("위치: X:%.2f, Y:%.2f, Z:%.2f", m_position.x, m_position.y, m_position.z);
			ImGui::Text("회전: X:%.2f, Y:%.2f, Z:%.2f", m_rotation.x, m_rotation.y, m_rotation.z);
			ImGui::Text("크기: X:%.2f, Y:%.2f, Z:%.2f", m_scale.x, m_scale.y, m_scale.z);

			ImGui::Text("local_TM", m_local._11, m_local._12, m_local._13, m_local._14);
			ImGui::Text("%.2f, %.2f, %.2f, %.2f", m_local._11, m_local._12, m_local._13, m_local._14);
			ImGui::Text("%.2f, %.2f, %.2f, %.2f", m_local._21, m_local._22, m_local._23, m_local._24);
			ImGui::Text("%.2f, %.2f, %.2f, %.2f", m_local._31, m_local._32, m_local._33, m_local._34);
			ImGui::Text("%.2f, %.2f, %.2f, %.2f\n", m_local._41, m_local._42, m_local._43, m_local._44);

			ImGui::Text("버텍스 갯수: %d", vertexArray.size());
			ImGui::Text("인덱스 갯수: %d", indexArray.size());
			ImGui::Text("삼각형어쩌고 갯수: %d", m_kuramonTriangleVertices.size());

			ImGui::End();
		});
}

void Kuramon::OnRender(const ComPtr<ID3D12GraphicsCommandList>& commadList)
{
	// 6. 그리기 전 세팅
	commadList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commadList->IASetVertexBuffers(0, 1, &m_kuramonVertexBufferView);
	commadList->IASetIndexBuffer(&m_kuramonIndexBufferView);
	commadList->SetGraphicsRootConstantBufferView(0, m_kuramonConstantBuffer->GetGPUVirtualAddress());

	// 7. 그리기 명령
	commadList->DrawIndexedInstanced(indexArray.size(), 1, 0, 0, 0);
}

void Kuramon::CreateConstantBuffer()
{
	// CBV디스크립터힙 heapProps
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 0;
	heapProps.VisibleNodeMask = 0;

	// 리소스 description
	D3D12_RESOURCE_DESC cbDesc = {};
	cbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	cbDesc.Alignment = 0;
	cbDesc.Width = 256; // 최소 256바이트(행렬 64 + 패딩)
	cbDesc.Height = 1;
	cbDesc.DepthOrArraySize = 1;
	cbDesc.MipLevels = 1;
	cbDesc.Format = DXGI_FORMAT_UNKNOWN;
	cbDesc.SampleDesc.Count = 1;
	cbDesc.SampleDesc.Quality = 0;
	cbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	cbDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// CB 리소스생성
	HRESULT hr = m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&cbDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_kuramonConstantBuffer)
	);
	ThrowIfFailed(hr);

	// CBV 디스크립터 힙 생성
	D3D12_DESCRIPTOR_HEAP_DESC heapDescCBV = {};
	heapDescCBV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // Constant Buffer View, Shader Resource View, Unordered Access View
	heapDescCBV.NumDescriptors = 1; // CBV 하나만 사용
	heapDescCBV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // 셰이더에서 접근 가능하도록 설정
	heapDescCBV.NodeMask = 0; // 멀티 GPU 시스템에서 사용할 노드 마스크, 단일 GPU 시스템에서는 1로 설정

	m_device->CreateDescriptorHeap(&heapDescCBV, IID_PPV_ARGS(&m_kuramonCBVHeap));

	// CBV 디스크립터 생성
	m_kuramonCBVDesc.BufferLocation = m_kuramonConstantBuffer->GetGPUVirtualAddress(); // CB 리소스의 GPU 가상 주소
	m_kuramonCBVDesc.SizeInBytes = (sizeof(Graphics::Matrix4x4) + 255) & ~255; // CBV는 256바이트 정렬이 필요하므로, 크기를 256바이트로 올림 처리

	m_kuramonCBVHandle = m_kuramonCBVHeap->GetCPUDescriptorHandleForHeapStart();
	m_device->CreateConstantBufferView(&m_kuramonCBVDesc, m_kuramonCBVHandle);

	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();

	DirectX::XMMATRIX mvp = world;


	Graphics::Matrix4x4 cbData = {};
	DirectX::XMStoreFloat4x4(&cbData.matrix, DirectX::XMMatrixTranspose(mvp)); // HLSL에서 row-major면 Transpose

	void* pData = nullptr;
	D3D12_RANGE readRange = { 0, 0 };
	ThrowIfFailed(m_kuramonConstantBuffer->Map(0, &readRange, &pData));
	memcpy(pData, &cbData, sizeof(Graphics::Matrix4x4));
}
