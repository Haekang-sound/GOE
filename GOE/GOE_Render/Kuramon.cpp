#include "Renderer_pch.h"
#include "Kuramon.h"
#include "../GOE_Core/Commons.h"
#include "../GOE_Editor/DebugManager.h"

// 디버거를 위해 추가하는 헤더이지만
// 나중에는 반드시 분리되어야함
#include "../Imgui/imgui.h"

Kuramon::Kuramon(){}

Kuramon::Kuramon(ComPtr<ID3D12Device> device, float aspectRatio)
	: m_device(device), m_aspectRatio(aspectRatio)
{
	m_kuramonMeshData = new MeshData();
}

Kuramon::~Kuramon()
{
}

void Kuramon::InitKuramon()
{
	m_local._11 = 1;
	m_local._22 = 1;
	m_local._33 = 1;
	m_local._44 = 1;
}

void Kuramon::LoadKuramon()
{
	/*CreateVertexBuffer();
	SetVertexBufferView();
	CreateIndexBuffer();
	CreateConstantBuffer();*/
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

void Kuramon::CreateVertexBuffer()
{

	//for (int i = 0; i < m_kuramonTriangleVertices.size(); ++i)
	for(const auto& v : vertexArray)
	{
		/// 일단 vertexArray를 외부에서 받아와야한다.
		// 벡터니까 이렇게 써줘야지
		m_kuramonTriangleVertices.push_back(v);
	}

	// 버텍스버퍼의 크기를 계산합니다.
	m_vertexBufferSize = sizeof(Vertex) * m_kuramonTriangleVertices.size();


	// 1. 디폴트 힙 리소스 생성
	D3D12_HEAP_PROPERTIES defaultHeapProps = {};
	defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = m_vertexBufferSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, // 일단 복사 대상으로 생성
		nullptr,
		IID_PPV_ARGS(&m_kuramonVertexBufferDefault)
	));

	// D3D12_HEAP_PROPERTIES
	// : 힙의 속성을 정의하는 구조체입니다.
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD; 					// 리소스를 할당할 힙의 속성을 정의합니다. 

	// D3D12_RESOURCE_DESC
	// : 리소스의 속성을 정의하는 구조체입니다.
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;// 버퍼 리소스입니다.
	resDesc.Alignment = 0;								// 정렬은 0으로 설정합니다.(자동설정됨)
	resDesc.Width = m_vertexBufferSize;					// 버퍼라면 바이트 크기, 텍스쳐면 x축 픽셀 수
	resDesc.Height = 1;									// 텍스쳐의 높이, 버퍼일 경우 높이는 1로 설정
	resDesc.DepthOrArraySize = 1;						// 깊이 또는 배열 크기
	resDesc.MipLevels = 1;								// 밉맵레벨 수, 버퍼는1 (1이면 밉맵없음)
	resDesc.Format = DXGI_FORMAT_UNKNOWN;				// 버퍼는 포맷이 필요 없습니다.
	resDesc.SampleDesc.Count = 1;						// 샘플링은 1로 설정
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;	// 버퍼는 반드시 ROW_MAJOR로 설정합니다.
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;			// 리소스 플래그는 없습니다.

	// CreateCommittedResource()
	// : 커밋된 리소스를 생성하는 메서드입니다.
	// 이 메서드는 힙과 리소스를 동시에 생성합니다.
	/*“커밋된(Committed)” 리소스란,
		리소스를 생성할 때 힙(메모리 공간)도 자동으로 같이 만들어서
		리소스와 힙이 1:1로 매칭되는 가장 단순한 형태.*/
		//	반대되는 개념 : “Placed Resource”
	ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProps,				// 힙 속성
		D3D12_HEAP_FLAG_NONE,	// 힙 플래그, 일반적으로 D3D12_HEAP_FLAG_NONE 사용
		&resDesc,				// 리소스 설명
		D3D12_RESOURCE_STATE_GENERIC_READ, // 리소스 생성 직후의 상태
		nullptr,				// 최적화된 클리어 값 포인터(텍스처, RTV, DSV 등만 해당) 일반 버퍼는 nullptr
		IID_PPV_ARGS(&m_kuramonVertexBufferUpload))); // 반환될 인터페이스의 ID
}
void Kuramon::SetVertexBufferView()
{
	// 업로드 힙에 정점 데이터를 복사하기 위해
// 업로드 힙의 시작 주소를 가져옵니다.
	UINT8* pVertexDataBegin;

	// D3D12_RANGE
	// : 업로드 힙의 데이터를 CPU가 읽을 수 있도록 매핑할 때 사용하는 구조체입니다.
	// Map() 호출 시 : CPU가 실제로 "읽을 범위"를 지정(읽을 게 없다면 {0, 0}로 설정)
	// Unmap() 호출 시:CPU가 실제로 "썼던 범위"를 지정
	D3D12_RANGE readRange = { 0,0 }; // 읽을 필요 없는 경우(주로 데이터 쓸 때)
	/*readRange가{ 0, 0 }이면
		GPU 드라이버는
		→ 메모리 캐시에서 버퍼 내용을 "CPU 쪽으로 읽어올 필요 없다"고 판단!
		→ memcpy로 쓰기만 할 테니 “최소한의 작업”만 해줌*/

		// Map() 메서드는 업로드 힙의 데이터를 CPU가 읽을 수 있도록 매핑합니다.
	ThrowIfFailed(m_kuramonVertexBufferUpload->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	// memcpy() 함수를 사용하여 정점 데이터를 업로드 힙에 복사합니다.
	
	memcpy(pVertexDataBegin, &m_kuramonTriangleVertices, m_vertexBufferSize);
	// Unmap() 메서드는 업로드 힙의 매핑을 해제합니다.
	m_kuramonVertexBufferUpload->Unmap(0, nullptr);




	// D3D12_VERTEX_BUFFER_VIEW
	// : 정점 버퍼 뷰를 정의하는 구조체입니다.
	// 이후 DrawCall 시 이 정보를 넘김
	// 이 뷰는 GPU가 정점 데이터를 읽을 때 사용됩니다.
	m_kuramonVertexBufferView.BufferLocation = m_kuramonVertexBufferDefault->GetGPUVirtualAddress();	// GPU에서 읽을 정점버퍼 시작 주소, 정점 버퍼의 GPU 가상 주소
	m_kuramonVertexBufferView.StrideInBytes = sizeof(Vertex);		// 정점버퍼 전체 크기(바이트 단위)
	m_kuramonVertexBufferView.SizeInBytes = m_vertexBufferSize;	// 정점 하나당 크기(바이트 단위)

}
void Kuramon::CreateIndexBuffer()
{
	// 인덱스 배열도 마찬가지로 얻어와야한다.
	m_kuramonIndexBufferSize = sizeof(uint32_t)* indexArray.size();

	// 1. Default Heap (GPU)
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	
	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = m_kuramonIndexBufferSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_kuramonIndexBufferDefault)
	));

	// 2. Upload Heap (CPU)
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_kuramonIndexBufferUpload)
	));

	// 3. 데이터 복사
	UINT8* pIndexDataBegin;
	D3D12_RANGE readRange = { 0, 0 };
	ThrowIfFailed(m_kuramonIndexBufferUpload->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));
	memcpy(pIndexDataBegin, &indexArray, m_kuramonIndexBufferSize);
	m_kuramonIndexBufferUpload->Unmap(0, nullptr);

	// 6. 인덱스버퍼 뷰 생성
	m_kuramonIndexBufferView.BufferLocation = m_kuramonIndexBufferDefault->GetGPUVirtualAddress();
	// CreateIndexBuffer() 함수 내부 수정
	m_kuramonIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_kuramonIndexBufferView.SizeInBytes = m_kuramonIndexBufferSize;
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
	m_kuramonCBVDesc.SizeInBytes = (sizeof(MVP) + 255) & ~255; // CBV는 256바이트 정렬이 필요하므로, 크기를 256바이트로 올림 처리

	m_kuramonCBVHandle = m_kuramonCBVHeap->GetCPUDescriptorHandleForHeapStart();
	m_device->CreateConstantBufferView(&m_kuramonCBVDesc, m_kuramonCBVHandle);

	DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();
	
	DirectX::XMMATRIX mvp = world;

	MVP cbData = {};
	DirectX::XMStoreFloat4x4(&cbData.mvp, DirectX::XMMatrixTranspose(mvp)); // HLSL에서 row-major면 Transpose

	void* pData = nullptr;
	D3D12_RANGE readRange = { 0, 0 };
	ThrowIfFailed(m_kuramonConstantBuffer->Map(0, &readRange, &pData));
	memcpy(pData, &cbData, sizeof(MVP));
}
void Kuramon::CopyUploadHeapToDefault(const ComPtr<ID3D12GraphicsCommandList>& commadList)
{
	commadList->CopyBufferRegion(
		m_kuramonVertexBufferDefault.Get(), 0,	// Dest
		m_kuramonVertexBufferUpload.Get(), 0,	// Src
		m_vertexBufferSize				// Size
	);
	// (3) 상태변환: 복사에서 VertexBuffer로 전환
	D3D12_RESOURCE_BARRIER vsBarrier = {};
	vsBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	vsBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	vsBarrier.Transition.pResource = m_kuramonVertexBufferDefault.Get();
	vsBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	vsBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	vsBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commadList->ResourceBarrier(1, &vsBarrier);

	// 인덱스 버퍼도 동일한 방식으로 복사합니다.
	commadList->CopyBufferRegion(m_kuramonVertexBufferDefault.Get(), 0, m_kuramonVertexBufferUpload.Get(), 0, m_kuramonIndexBufferSize);

	// 5. 상태변환
	D3D12_RESOURCE_BARRIER ibBarrier = {};
	ibBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	ibBarrier.Transition.pResource = m_kuramonIndexBufferDefault.Get();
	ibBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	ibBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
	ibBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	commadList->ResourceBarrier(1, &ibBarrier);
}

void Kuramon::DirextXVertexToKuramonVertex()
{
	Vertex vetex;
	for(const auto& v: m_kuramonMeshData->vertices)
	{
		vetex.position.x = v.position[0];
		vetex.position.y = v.position[1];
		vetex.position.z = v.position[2];

		vertexArray.push_back(vetex);
	}
	for(const auto& i : m_kuramonMeshData->indices)
	{
		indexArray.push_back(i);
	}
}

