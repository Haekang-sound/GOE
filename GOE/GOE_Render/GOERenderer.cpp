#include "Renderer_pch.h"
#include "GOERenderer.h"

#include <d3dx12/d3dx12.h>
#include "DirectXTex.h"	
#include "dxcapi.h"


#include "SwapChain.h"

#include "Camera.h" 

#include "MeshResource.h"
#include "TextureResource.h"
#include "RenderObject.h"

#if defined(_DEBUG)
#include "PIX.h"
#endif

/// <summary>
/// GOERenderer의 생성자
/// 랜더러에 필요한 클래스들을 생성한다.
/// 
/// </summary>
/// <param name="hWnd">윈도우 핸들</param>
GOERenderer::GOERenderer(const HWND hWnd)
	: m_hWnd(hWnd)
{
	/// 분리했으니 제대로 생성해야한다.
	m_swapChain = std::make_unique<Graphics::SwapChain>(m_hWnd, 2);

	m_UIInitInfo = std::make_unique<UIInitInfo>();
	m_UILoopInfo = std::make_unique <UILoopInfo>();
}

/// <summary>
/// GOERenderer의 소멸자
/// 
/// </summary>
GOERenderer::~GOERenderer()
{
	OnDestroy();
}

/// <summary>
/// 초기화 함수
///
/// </summary>
void GOERenderer::OnInit()
{
	/// 순서는 중요합니다.
	GD::GetInstance().Initialize(false, true);	
#if defined(_DEBUG)
	GetLatestWinPixGpuCapturerPath_Cpp17();
	// Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
	// This may happen if the application is launched through the PIX UI. 
	if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
	{
		LoadLibrary(L"C:\\Program Files\\Microsoft PIX\\2509.25\\WinPixGpuCapturer.dll");
	}
#endif
	m_swapChain->Initialize();
	
	CreateCommandAllocator();
	m_camera = new Camera(m_hWnd);
	LoadAssets();
	CreateImguiDescriptorHeap();
}

void GOERenderer::OnUpdate()
{
	m_camera->OnUpdate();
	
	// 랜더오브젝트들을 그리는구간
	// 여긴 콘스탄트 버퍼를 업데이트 하는거임
	for (const auto& renderObject : m_renderObjects)
	{
		Graphics::CB cbData = {};
		XMMATRIX world = XMLoadFloat4x4(&renderObject.get()->GetLocalTM().matrix);
		XMMATRIX vp =
			m_camera->GetViewTransform()
			* XMLoadFloat4x4(&m_swapChain.get()->m_proj);
		cbData.cameraPosition = m_camera->GetPosition();
		
		XMStoreFloat4x4(&cbData.world, world);
		XMStoreFloat4x4(&cbData.viewProjection, vp);

		void* pData = nullptr;
		D3D12_RANGE readRange = { 0, 0 };
		ThrowIfFailed(renderObject->GetCB()->Map(0, &readRange, &pData));
		memcpy(pData, &cbData, sizeof(Graphics::CB));
		renderObject->GetCB()->Unmap(0, nullptr);

		/// boneMatrix에 본순서대로 업데이트된 메트릭스를 채우넣으면 된다.
		XMFLOAT4X4 boneMatrix[128];
		for (int i = 0; i < 128; ++i)
		{
			boneMatrix[i] = renderObject->GetBoneTM(i).matrix;
		}
		void* pBoneData = nullptr;
		D3D12_RANGE boneRange = { 0, 0 };
		ThrowIfFailed(renderObject->GetCBBoneMatrix()->Map(0, &boneRange, &pBoneData));
		memcpy(pBoneData, &boneMatrix, sizeof(XMFLOAT4X4)*128);
		renderObject->GetCB()->Unmap(0, nullptr);		
	}
}

/// <summary>
/// 그리기 전에 필요한 동작을 합니다.
/// 
/// </summary>
void GOERenderer::BeginRender()
{
	GD::GetInstance().WaitForFence(GD::GetInstance().m_fenceValue);

	// 1. 커맨드 할당자와 커맨드 리스트 초기화
	// 이전에 기록된 GPU 작업(커맨드 리스트)이 끝났으니, 새롭게 명령을 기록할 수 있도록 할당자(Allocator)를 리셋합니다.
	m_commandAllocator->Reset();
	// 커맨드 리스트(실제 명령 기록 객체)를 리셋하고, 새 명령을 이 할당자에, 지정한 파이프라인 상태(m_pipelineState)로 기록하겠다고 선언.
	m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());

	// 2. 그래픽스 파이프라인 세팅
		// 셰이더들이 쓸 수 있는 리소스(텍스처, 버퍼 등)들의 묶음인 Root Signature를 바인딩.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	// 뷰포트(화면에 그릴 영역의 크기와 위치, 카메라 뷰)를 지정.
	m_commandList->RSSetViewports(1, &m_swapChain.get()->m_viewport);
	// ScissorRECT 지정. 이 영역 바깥은 렌더링 안 함(클리핑).
	m_commandList->RSSetScissorRects(1, &m_swapChain.get()->m_scissorRect);

	/*1. 베리어(Barrier)란 ?
		GPU 리소스(버퍼, 텍스처 등)의 “상태 전환”을 명시적으로 선언하는 명령
		D3D12에서 리소스는 “읽기”, “쓰기”, “카피”, “표시(Present)”, “렌더타겟”, “셰이더리소스” 등 다양한 상태를 가짐
		GPU 파이프라인의 단계마다 리소스가 “올바른 상태”에 있어야만 GPU가 올바르게 처리함
		베리어는 “지금부터 이 리소스 상태를 바꾼다”를 GPU에 알려주는 명령어*/
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_swapChain.get()->m_renderTargets[m_swapChain.get()->m_frameIndex].Get(),    // pResource
		D3D12_RESOURCE_STATE_PRESENT,           // StateBefore
		D3D12_RESOURCE_STATE_RENDER_TARGET      // StateAfter
	);

	//3. 리소스 배리어(상태 변경) – “Present → RenderTarget”
		// 현재 그릴 렌더타겟(BackBuffer)의 상태를 “화면에 표시(PRESENT)” → “렌더링(RTT)” 상태로 전환
	m_commandList->ResourceBarrier(1, &barrier);

	/// 4. 렌더 타겟 뷰 바인딩 + DSV 핸들 추가 
	/// 드라마틱 한 변화이기 때문에 반드시 정리하자 
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapChain.get()->m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += m_swapChain.get()->m_frameIndex * m_swapChain.get()->m_rtvDescriptorSize;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_swapChain.get()->m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

	// Output Merger(최종 출력단)에 "이 렌더타겟에 그려라" 지정. 
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle); // DSV 핸들 추가

	// 5. 렌더 타겟 클리어(색상 초기화)
	const float clearColor[] = { .7f, .7f, .5f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr); // 깊이 버퍼 클리어
}

/// <summary>
/// 랜더링 루프를 담당합니다.
/// 
/// 이 함수는 커맨드 리스트를 채우고,
/// 커맨드 큐를 통해 실행하며,
/// 스왑체인을 통해 화면에 출력합니다.
/// </summary>
void GOERenderer::OnRender()
{
	/// 지금은 모든 모델을 그리지만 나중에는 선택적으로 그려야한다.
	/// 공유자원이 아닌 고유자원을 기준으로 랜더오브젝트의 관한 queue를 만들어야한다.
	/// 렌더오브젝트는 메쉬단위이므로 meshresource는 해쉬맵이어야한다.
	for (const auto& renderObject : m_renderObjects)
	{
		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->IASetVertexBuffers(0, 1, &m_meshResourceMap[renderObject->GetMeshID()]->GetVBView());
		m_commandList->IASetIndexBuffer(&m_meshResourceMap[renderObject->GetMeshID()]->GetIBView());

		/// 콘스탄트 버퍼의 관한 문제는 고유자원을 기준으로 랜더할때 해결될것
		// 1. 월드/뷰/프로젝션 CBV 바인딩 (루트 파라미터 0)
		m_commandList->SetGraphicsRootConstantBufferView(0, renderObject->GetCB()->GetGPUVirtualAddress());

		// 2. 본 변환 CBV 바인딩 (루트 파라미터 1) - RenderObject 소유
		//    RenderObject에 GetBoneCB() 와 같은 함수가 추가되어야 합니다.
		m_commandList->SetGraphicsRootConstantBufferView(1, renderObject->GetCBBoneMatrix()->GetGPUVirtualAddress()); // 예시: GetBoneCB() 호출

		// 3. 본 오프셋 CBV 바인딩 (루트 파라미터 2) - MeshResource 소유
		m_commandList->SetGraphicsRootConstantBufferView(2, m_meshResourceMap[renderObject->GetMeshID()]->GetCB()->GetGPUVirtualAddress()); // MeshResource의 CB 사용

		// 4. 텍스처 서술자 테이블 바인딩 (루트 파라미터 3)
		ID3D12DescriptorHeap* ppHeaps[] = { m_textureResourceMap[renderObject->GetTextureID()]->GetTextureHeap() }; // TextureResource에서 힙 가져오기
		m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps); // 힙 설정

		// GPU 핸들 가져오기 (TextureResource에 GetSRVGpuHandle() 같은 함수가 있으면 더 좋습니다)
		CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpuHandle(m_textureResourceMap[renderObject->GetTextureID()]->GetTextureHeap()->GetGPUDescriptorHandleForHeapStart());
		// 루트 파라미터 인덱스를 3으로 변경!
		m_commandList->SetGraphicsRootDescriptorTable(3, srvGpuHandle); // <--- 인덱스 3 사용

		// 7. 그리기 명령
		m_commandList->DrawIndexedInstanced(m_meshResourceMap[renderObject->GetMeshID()]->GetIndexCount(), 1, 0, 0, 0);
	}
	// 6. 그리기 전 세팅
}

/// <summary>
/// 그리고 나서 필요한 동작을 합니다.
/// 
/// </summary>
void GOERenderer::EndRender()
{
	// 8. 리소스 배리어(상태 변경) – “RenderTarget → Present”
	// 렌더링이 끝났으니, 다시 "화면에 표시(PRESENT)" 상태로 전환
	// 이 상태 변경은 GPU가 커맨드 리스트를 실행하는 동안 자동으로 처리됩니다.
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_swapChain.get()->m_renderTargets[m_swapChain.get()->m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,      // StateBefore (현재 상태)
		D3D12_RESOURCE_STATE_PRESENT             // StateAfter (목표 상태)
	);

	//3. 리소스 배리어(상태 변경) – “Present → RenderTarget”
		// 현재 그릴 렌더타겟(BackBuffer)의 상태를 “화면에 표시(PRESENT)” → “렌더링(RTT)” 상태로 전환
	m_commandList->ResourceBarrier(1, &barrier);

	// 9. 커맨드 리스트 닫기
		// 커맨드 리스트에 더 이상 명령을 추가하지 않겠다고 선언합니다.
		// 이 메서드를 호출한 후에는 커맨드 리스트를 실행할 수 있습니다.
	m_commandList->Close();

	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	GD::GetInstance().m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_swapChain.get()->m_swapChain->Present(1, 0);

	GD::GetInstance().m_fenceValue++;
	GD::GetInstance().SignalFence(GD::GetInstance().m_fenceValue);

	// GPU 작업이 끝났으니, swapchain에서 새로운 백버퍼 인덱스를 받아옴.
	m_swapChain.get()->m_frameIndex = m_swapChain.get()->m_swapChain->GetCurrentBackBufferIndex();

}

/// <summary>
/// 랜더러의 자원을 해제합니다.
/// 
/// </summary>
void GOERenderer::OnDestroy()
{
	GD::GetInstance().WaitForFence(GD::GetInstance().m_fenceValue);
	CloseHandle(GD::GetInstance().m_fenceEvent);
	delete m_camera;
}

UIInitInfo* GOERenderer::GetUIInfo()
{
	m_UIInitInfo.get()->commandQueue = GD::GetInstance().m_commandQueue.Get();
	m_UIInitInfo.get()->device = GD::GetInstance().m_device.Get();
	m_UIInitInfo.get()->frameBufferCount = m_swapChain.get()->m_frameBufferCount;
	m_UIInitInfo.get()->imguiDescriptorHeap = m_imguiDescriptorHeap.Get();
	return m_UIInitInfo.get();
}

UILoopInfo* GOERenderer::GetUILoopInfo()
{
	m_UILoopInfo.get()->commandlist = m_commandList.Get();
	m_UILoopInfo.get()->imguiDescriptorHeap = m_imguiDescriptorHeap.Get();
	m_UILoopInfo.get()->rendertarget = m_swapChain.get()->m_renderTargets[m_swapChain.get()->m_frameIndex].Get();
	return m_UILoopInfo.get();
}

/// <summary>
/// 랜더오브젝트를 생성한다.
/// </summary>
void GOERenderer::AddRenderObejct(RenderObjectData& data)
{
	auto newrendrobj = std::make_unique<RenderObject>(data);
	m_renderObjects.emplace_back(std::move(newrendrobj));
	// 콘스탄트버퍼를 개별적으로 생성해준다.
	CreateRenderObjectCBResource(m_renderObjects.back().get());
}


/// <summary>
/// 셰이더와 파이프라인 상태를 컴파일하고 생성합니다.
/// 
/// 정점정보와 셰이더를 컴파일하고,
/// 파이프라인 상태를 생성하여
/// 랜더링 파이프라인을 설정합니다.
/// 
/// </summary>
void GOERenderer::LoadAssets()
{
	CreateRootSignature();
	CompileShaders();
	CreatePipelineState();
	CreateCommandList();
}

/// <summary>
/// 스왑체인을 생성하고 
/// 현재 백버퍼 인덱스를 설정합니다.
/// </summary>
/// <returns></returns>


/// <summary>
/// 커맨드 얼로케이터를 생성합니다.
/// 
/// </summary>
/// <returns></returns>
void GOERenderer::CreateCommandAllocator()
{
	ThrowIfFailed(GD::GetInstance().m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

/// <summary>
/// 루트시그니처를 생성합니다.
/// 
/// 루트시그니처는 파이프라인 상태를 설정하는데 사용되며,
/// 입력 어셈블러 단계에서 사용되는 리소스의 바인딩을 정의합니다.
/// </summary>
/// <returns></returns>
void GOERenderer::CreateRootSignature()
{
	// 루트시그니처는
	// 셰이더가 사용하는 리소스(버퍼, 텍스처 등)의 바인딩을 정의합니다.
	// 셰이더마다 구성이 다를 경우
	// 셰이더마다 루트 시그니처를 따로 정의해야 합니다.
	// D3D12_ROOT_SIGNATURE_DESC는
	// 루트 시그니처의 속성을 정의하는 구조체입니다.
	// 루트시그니처의 형태 == 셰이더가 사용하는 리소스의 형태와 일치해야 합니다.

	// D3D12_ROOT_SIGNATURE_DESC
	// : 루트 시그니처의 속성을 정의하는 구조체입니다.
	// NumParameters : 루트 시그니처에 포함될 파라미터의 수
	// pParameters : 루트 시그니처 파라미터 배열
	// NumStaticSamplers : 정적 샘플러의 수
	// pStaticSamplers : 정적 샘플러 배열
	// Flags : 루트 시그니처의 플래그를 지정합니다.

	D3D12_DESCRIPTOR_RANGE descriptorRanges[1];
	descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRanges[0].NumDescriptors = 1;    // 텍스처 1개
	descriptorRanges[0].BaseShaderRegister = 0;    // 셰이더의 t0 레지스터에 바인딩
	descriptorRanges[0].RegisterSpace = 0;
	descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	// D3D12_ROOT_PARAMETER
	// : 루트 시그니처의 파라미터를 정의하는 구조체입니다.
	// --- 루트 파라미터 배열 선언 (한 번만!) ---
	D3D12_ROOT_PARAMETER rootParameters[4] = {}; // 총 4개의 파라미터 (CBV 3개 + Table 1개)

	// 파라미터 0: 월드/뷰/투영 CBV (b0) - RenderObject 소유
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].Descriptor.ShaderRegister = 0; // b0 레지스터
	rootParameters[0].Descriptor.RegisterSpace = 0;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // 모든 셰이더 단계에서 접근 가능

	// 파라미터 1: 본 변환 행렬 CBV (b1) - RenderObject 소유
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].Descriptor.ShaderRegister = 1; // b1 레지스터
	rootParameters[1].Descriptor.RegisterSpace = 0;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // 버텍스 셰이더에서만 필요

	// 파라미터 2: 본 오프셋 행렬 CBV (b2) - MeshResource 소유
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[2].Descriptor.ShaderRegister = 2; // b2 레지스터
	rootParameters[2].Descriptor.RegisterSpace = 0;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // 버텍스 셰이더에서만 필요

	// 파라미터 3: 텍스처 SRV 서술자 테이블 (t0)
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].DescriptorTable.NumDescriptorRanges = 1; // 범위 1개
	rootParameters[3].DescriptorTable.pDescriptorRanges = descriptorRanges; // 위에서 정의한 범위 사용
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // 픽셀 셰이더에서만 필요


	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0; // s0
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;



	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = 4;
	rootSignatureDesc.pParameters = rootParameters;
	rootSignatureDesc.NumStaticSamplers = 1;
	rootSignatureDesc.pStaticSamplers = &sampler;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	ThrowIfFailed(GD::GetInstance().m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

}


HRESULT GOERenderer::CompileShaderFromFile(
	const WCHAR * fileName,     
	const WCHAR * entryPoint,
	const WCHAR * targetProfile,
	ID3DBlob * *ppShaderBlob)
{
	ComPtr<IDxcUtils> pUtils;
	ComPtr<IDxcCompiler3> pCompiler;
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
	if (FAILED(hr)) return hr;
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));
	if (FAILED(hr)) return hr;

	ComPtr<IDxcBlobEncoding> pSource;
	hr = pUtils->LoadFile(fileName, nullptr, &pSource);
	if (FAILED(hr))
	{
		OutputDebugStringW(L"Failed to load shader file: ");
		OutputDebugStringW(fileName);
		OutputDebugStringW(L"\n");
		return hr;
	}

	// 컴파일 옵션 설정
	std::vector<LPCWSTR> arguments;

	// 원본 파일 경로를 가장 먼저 추가 (PDB 내 원본 경로 기록용)
	arguments.push_back(fileName);

	// 엔트리 포인트 및 타겟 프로필
	arguments.push_back(L"-E");
	arguments.push_back(entryPoint);
	arguments.push_back(L"-T");
	arguments.push_back(targetProfile);

	// 일반 옵션
	arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);

	// PDB 파일 경로를 저장할 변수
	std::wstring pdbPath; // 최종 절대 경로 저장

#if defined(_DEBUG)
	// 디버그 빌드 옵션
	arguments.push_back(L"-Zi");
	arguments.push_back(L"-Od");
	arguments.push_back(L"-Qembed_debug"); // 선택 사항

	// --- 절대 PDB 경로 설정 ---
	// 1. 지정된 절대 경로 사용
	std::filesystem::path pdbSaveDir = L"D:\\project\\GOE\\GOE\\Bin\\Debug\\x64\\exe";

	// 2. PDB 파일 이름 생성 (기존 코드 활용)
	std::filesystem::path shaderFilePath = fileName;
	std::wstring pdbName = shaderFilePath.filename().wstring();
	size_t dot = pdbName.rfind(L'.');
	if (dot != std::wstring::npos)
	{
		pdbName = pdbName.substr(0, dot);
	}
	pdbName += L".pdb"; // 예: "shader_vs.pdb"

	// 3. 최종 절대 PDB 경로 조합
	std::filesystem::path fullPdbPath = pdbSaveDir / pdbName; // 예: D:\project\GOE\GOE\Bin\Debug\x64\exe\shader_vs.pdb
	pdbPath = fullPdbPath.wstring(); // std::wstring으로 변환

	// 4. -Fd 인수로 전달
	arguments.push_back(L"-Fd");
	arguments.push_back(pdbPath.c_str()); // 계산된 절대 경로 전달
	// --- 절대 PDB 경로 설정 끝 ---

#else
	// 릴리즈 빌드 옵션
	arguments.push_back(L"-O3");
	arguments.push_back(L"-Qstrip_reflect");
	arguments.push_back(L"-Qstrip_debug");
#endif

	DxcBuffer sourceBuffer;
	sourceBuffer.Ptr = pSource->GetBufferPointer();
	sourceBuffer.Size = pSource->GetBufferSize();
	sourceBuffer.Encoding = 0; // 필요시 DXC_CP_UTF8 등으로 변경

	ComPtr<IDxcResult> pResult;
	hr = pCompiler->Compile(
		&sourceBuffer,
		arguments.data(),
		static_cast<UINT32>(arguments.size()),
		nullptr, // Include handler
		IID_PPV_ARGS(&pResult)
	);

	if (FAILED(hr))
	{
		OutputDebugStringW(L"DXC Compile() failed.\n");
		return hr;
	}

	ComPtr<IDxcBlobUtf8> pErrors;
	hr = pResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
	if (SUCCEEDED(hr) && pErrors != nullptr && pErrors->GetStringLength() > 0)
	{
		OutputDebugStringA("Shader Compiler Errors:\n");
		OutputDebugStringA(pErrors->GetStringPointer());
		OutputDebugStringA("\n");
	}

	HRESULT compileStatus;
	hr = pResult->GetStatus(&compileStatus);
	if (FAILED(hr) || FAILED(compileStatus))
	{
		OutputDebugStringW(L"Shader compilation failed.\n");
		return FAILED(compileStatus) ? compileStatus : hr;
	}

	// 컴파일 성공: PDB 저장 및 셰이더 바이트코드 반환
#if defined(_DEBUG)
	ComPtr<IDxcBlob> pPdbBlob;
	hr = pResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPdbBlob), nullptr);

	if (SUCCEEDED(hr) && pPdbBlob != nullptr)
	{
		// -Fd 인수로 지정한 절대 경로(pdbPath)에 PDB 파일 저장
		FILE* fp = nullptr;
		errno_t err = _wfopen_s(&fp, pdbPath.c_str(), L"wb"); // 절대 경로 사용
		if (err == 0 && fp != nullptr)
		{
			fwrite(pPdbBlob->GetBufferPointer(), pPdbBlob->GetBufferSize(), 1, fp);
			fclose(fp);
			OutputDebugStringW((L"Saved PDB: " + pdbPath + L"\n").c_str());
		}
		else
		{
			OutputDebugStringW((L"Failed to save PDB: " + pdbPath + L"\n").c_str());
			// 권한 문제 등이 의심될 경우 에러 코드 출력 추가 가능
		}
	}
	else
	{
		OutputDebugStringW(L"Failed to get PDB output from DXC result.\n");
	}
#endif

	hr = pResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(reinterpret_cast<IDxcBlob**>(ppShaderBlob)), nullptr);
	if (FAILED(hr))
	{
		OutputDebugStringW(L"Failed to get Shader Object output from DXC result.\n");
	}

	return hr;
}

/// <summary>
/// 경로를 지정하여 셰이더를 컴파일합니다.
/// 
/// </summary>
/// <returns></returns>
void GOERenderer::CompileShaders()
{
	// 픽셀 셰이더 컴파일
	ThrowIfFailed(CompileShaderFromFile(
		L"..\\Shader\\shader_ps.hlsl",
		L"Main",
		L"ps_6_0", // 셰이더 모델 6.0 이상 권장
		m_pixelShader.GetAddressOf()
	));

	// 버텍스 셰이더 컴파일
	ThrowIfFailed(CompileShaderFromFile(
		L"..\\Shader\\shader_vs.hlsl",
		L"Main",
		L"vs_6_0", // 셰이더 모델 6.0 이상 권장
		m_vertexShader.GetAddressOf()
	));

}

/// <summary>
/// 파이프라인상태객체(PSO)를 생성합니다.
/// 
/// </summary>
/// <returns></returns>
void GOERenderer::CreatePipelineState()
{
	// D3D12_INPUT_ELEMENT_DESC
	// : 입력 어셈블러 단계에서 사용되는 입력 레이아웃을 정의합니다.
	D3D12_INPUT_ELEMENT_DESC iaDesc;
	iaDesc = {
		"POSITION",						// SemanticName			: 입력 레이아웃의 세맨틱 이름입니다.
		0,								// SemanticIndex		: 세맨틱 인덱스, 같은 이름을 가진 여러 데이터가 있을 때 구분해주는 번호.
		DXGI_FORMAT_R32G32B32_FLOAT,	// Format				: 입력 데이터의 데이터 포맷(타입, 크기).
		0,								// InputSlot			: 어떤 입력 버퍼에서 이 데이터를 읽을지 지정.
		0,								// AlignedByteOffset	: 이 데이터가 구조체 내에서 몇 바이트 떨어져 있는지 오프셋
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, // InputSlotClass : 정점 데이터냐, 인스턴스 데이터냐 구분
		0								// InstanceDataStepRate	: 인스턴스 데이터 단계 속도입니다. 인스턴스 데이터의 경우, 몇 번 정점마다 값을 갱신할지.
	};
	m_inputElementDescs[0] = iaDesc;
	iaDesc = { "COLOR",
		0,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		0,
		D3D12_APPEND_ALIGNED_ELEMENT,// 앞에거 바로뒤에 온다는 뜻
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		0 };
	m_inputElementDescs[1] = iaDesc;

	// 텍스처 좌표를 위한 입력 레이아웃 정의
	iaDesc = {
		"TEXCOORD",
		0,
		DXGI_FORMAT_R32G32_FLOAT,
		0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		0 };
	m_inputElementDescs[2] = iaDesc;

	// 텍스처 좌표를 위한 입력 레이아웃 정의
	iaDesc = {
		"NORMAL",
		0,
		DXGI_FORMAT_R32G32B32_FLOAT,
		0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		0 };
	m_inputElementDescs[3] = iaDesc;
	// 4개의 unsigned int를 한 묶음으로 보냅니다.
	iaDesc = { "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	m_inputElementDescs[4] = iaDesc;

	// 4개의 float를 한 묶음으로 보냅니다.
	iaDesc = { "BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	m_inputElementDescs[5] = iaDesc;

	// D3D12_SHADER_BYTECODE
	// : 셰이더 코드의 바이트코드를 나타내는 구조체입니다.
	// 이 구조체는 파이프라인 상태 객체(PSO)를 생성할 때 사용됩니다.
	D3D12_SHADER_BYTECODE VSTemp = {};
	VSTemp.pShaderBytecode = m_vertexShader.Get()->GetBufferPointer();
	VSTemp.BytecodeLength = m_vertexShader.Get()->GetBufferSize();
	D3D12_SHADER_BYTECODE PSTmp = {};
	PSTmp.pShaderBytecode = m_pixelShader.Get()->GetBufferPointer();
	PSTmp.BytecodeLength = m_pixelShader.Get()->GetBufferSize();

	// D3D12_RASTERIZER_DESC
	// : 래스터라이저 상태를 정의하는 구조체입니다.
	D3D12_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;	// FillMode : 삼각형의 면을 어떻게 채울지 지정, D3D12_FILL_MODE_SOLID: 면을 꽉 채움(기본값), D3D12_FILL_MODE_WIREFRAME: 와이어프레임(테두리만 그림, 디버깅 등에서 유용)
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;		// CullMode : 백페이스 컬링(Backface Culling) 사용 여부 및 방식
	rasterizerDesc.FrontCounterClockwise = FALSE;		// FrontCounterClockwise : 삼각형의 "앞면"을 어떤 방향의 버텍스 나열로 정의할지TRUE: 반시계 방향이 앞면, FALSE: 시계 방향이 앞면(기본값)
	rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;// DepthBias : 깊이 편향(Depth Bias) 값, 깊이 테스트 시 삼각형의 깊이를 조정하는 데 사용, z-파이팅(z-fighting) 현상을 줄이기 위한 깊이 오프셋(정수)
	rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;	// DepthBiasClamp : DepthBias가 적용될 수 있는 최대/최소치(절댓값), 0이면 제한없음
	rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS; // SlopeScaledDepthBias : 깊이 편향을 삼각형의 기울기에 따라 조정하는 값, 기울기가 클수록 깊이 편향이 커짐
	rasterizerDesc.DepthClipEnable = TRUE;				// DepthClipEnable : 깊이(Z) 값이 뷰 프러스텀 바깥일 때, 잘라낼지(클리핑) 여부
	rasterizerDesc.MultisampleEnable = FALSE;			// MultisampleEnable : 멀티샘플링(MSAA) 사용 여부
	rasterizerDesc.AntialiasedLineEnable = FALSE;		// AntialiasedLineEnable : 라인(선) 렌더링 시 안티에일리어싱 적용 여부
	rasterizerDesc.ForcedSampleCount = 0;				// 래스터라이저가 강제로 쓸 샘플의 개수, 보통 0(기본값)
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF; // ConservativeRaster : 충돌체크, 물리엔진 등 특수한 경우에만 씀

	/*블렌드(Blend)란 ?
		블렌딩은
		픽셀을 그릴 때 "지금 그리려는 픽셀"과 "이미 화면에 있는 픽셀(배경 픽셀)"을
		어떤 방식으로 섞어서(blend) 최종 색을 만들지 정하는 과정.*/

		// D3D12_BLEND_DESC
		// : 블렌드 상태를 정의하는 구조체입니다.
	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;	// AlphaToCoverageEnable : 알파 투 커버리지(Alpha to Coverage) 사용 여부, 알파값을 멀티샘플링 커버리지 마스크에 적용해서, 부드러운 반투명/투명 효과를 낼 수 있음 (주로 vegetation, 트리 리프, 파티클 등에 씀)
	blendDesc.IndependentBlendEnable = FALSE;	// IndependentBlendEnable : 여러 렌더 타겟을 사용할 때, 각 타겟마다 블렌드 상태를 다르게 할지 여부

	// RenderTarget[8] : 최대 8개의 렌더타겟 각각에 대한 블렌딩 설정
	for (int i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)	// D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT : 동시에 렌더링할 수 있는 최대 렌더 타겟 개수
	{
		blendDesc.RenderTarget[i].BlendEnable = TRUE;				// 블랜딩 사용여부
		blendDesc.RenderTarget[i].LogicOpEnable = FALSE;			// 논리연산(AND/OR/XOR 등)을 블렌드 대신 사용여부
		blendDesc.RenderTarget[i].SrcBlend = D3D12_BLEND_SRC_ALPHA;		// 소스(새로 그릴 픽셀)의 색상에 곱할 계수
		blendDesc.RenderTarget[i].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;		// 대상(이미 화면에 있는 픽셀)의 색상에 곱할 계수
		blendDesc.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;		// 블렌드 연산 종류
		blendDesc.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;	// 소스 알파값에 곱할 계수
		blendDesc.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;// 대상(화면에 있던 픽셀)의 알파값에 곱할 계수
		blendDesc.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;// 알파 블렌드 연산 종류
		blendDesc.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_NOOP;	// 논리 연산 종류
		blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // 해당 렌더타겟의 R/G/B/A 채널 중 어느 것에 쓸지 비트마스크로 지정
	}

	// D3D12_GRAPHICS_PIPELINE_STATE_DESC
	// : 그래픽스 파이프라인 상태를 정의하는 구조체입니다.
	// <> 필수 표시
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_rootSignature.Get();	// 루트 시그니처를 설정합니다.<>
	psoDesc.VS = VSTemp;	// 정점 셰이더를 설정합니다.<>
	psoDesc.PS = PSTmp;		// 픽셀 셰이더를 설정합니다.<>
	psoDesc.DS;				// 도메인 셰이더
	psoDesc.HS;				// 헐 셰이더
	psoDesc.GS;				// 지오메트리 셰이더
	psoDesc.StreamOutput;	// 스트림 아웃(geometry shader 이후 데이터를 버퍼에 기록)
	psoDesc.BlendState = blendDesc;					// 블렌드 상태를 설정합니다.<>
	psoDesc.SampleMask = UINT_MAX;					// 샘플 마스크를 설정합니다. 각 멀티샘플 픽셀(멀티샘플링/MSAA)마다 어떤 샘플에만 쓰기를 허용할지 마스킹, 대부분의 경우 UINT_MAX(모든 샘플에 기록) 사용
	psoDesc.RasterizerState = rasterizerDesc;		// 래스터라이저 상태를 설정합니다.<>
	psoDesc.DepthStencilState.DepthEnable = TRUE; // 깊이 테스트 활성화
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; // 새로 그릴 픽셀이 깊이테스트를 통과하면 그린다.
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS; // 카메라와 가까이 있는것을 그린다는 옵션
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.InputLayout = { m_inputElementDescs, _countof(m_inputElementDescs) };	// 입력 레이아웃을 설정합니다. 정점 버퍼에서 셰이더로 보낼 데이터의 포맷/구조(어떤 데이터가 어디에 있는지)
	psoDesc.IBStripCutValue;														// 인덱스 버퍼 스트립 컷 값, 프리미티브 스트립(예: 삼각형 스트립)에서 "컷"으로 쓸 특별한 인덱스 값(primitive restart)
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;			// 프리미티브 타입(기본 도형 종류)	드로우콜에서 어떤 도형을 그릴지 D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE(삼각형)	D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE(선)
	psoDesc.NumRenderTargets = 1;						// 동시에 출력할 Render Target 개수
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // 렌더 타겟의 포맷을 설정합니다.<>
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT; // DSV 포맷 설정

	psoDesc.SampleDesc.Count = 1;	// 샘플링 개수, 멀티샘플링(MSAA) 사용 여부를 결정합니다. 1이면 MSAA 사용 안함, 2 이상이면 MSAA 사용
	psoDesc.NodeMask;				// 멀티 GPU 시스템에서 파이프라인 상태가 실행될 노드 마스크를 지정합니다. (멀티 GPU 시스템에서만 사용, 단일 GPU 시스템에서는 0)
	psoDesc.CachedPSO;				// 캐시된 파이프라인 상태 객체(PSO)를 설정합니다. 파이프라인 상태 캐시(빠른 생성, 로딩 지원용)
	psoDesc.Flags;					// 추가 플래그(특별한 최적화 옵션 등)



	/*랜더타겟 포멧 여러번 지정하는 이유
		텍스처(리소스) 만들 때 포맷 지정
		⇒ 실제 픽셀 데이터의 타입과 구조

		RTV 만들 때 포맷 지정
		⇒ 이 버퍼를 “렌더타겟 뷰”로 쓸 때 쓸 포맷(일반적으로 리소스 포맷과 같아야 함)

		PSO(파이프라인) 만들 때 RTVFormats 지정
		⇒ 앞으로 파이프라인에서 렌더타겟으로 쓸 뷰들의 포맷을 명확히 선언

		이 세 단계가 다 일치해야만 Direct3D 12가 파이프라인을 올바르게 동작시킴*/


		// CreateGraphicsPipelineState()
		// : 그래픽스 파이프라인 상태 객체(PSO)를 생성합니다.
	ThrowIfFailed(GD::GetInstance().m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

/// <summary>
/// 커맨드리스트를 생성합니다.
/// 
/// </summary>
/// <returns></returns>
void GOERenderer::CreateCommandList()
{
	// D3D12_COMMAND_LIST_TYPE_DIRECT
	// : 이 타입은 GPU에 직접 명령을 보내는 커맨드 리스트를 생성합니다.
	ThrowIfFailed(GD::GetInstance().m_device->CreateCommandList(
		0,								// NODMASK
		D3D12_COMMAND_LIST_TYPE_DIRECT, // TYPE : 커맨드리스트 타입(무엇을 기록할지 종류) 지정
		m_commandAllocator.Get(),		// CommandAllocator : 커맨드리스트와 1:1 매핑은 아님! (여러 번 재사용 가능)
		m_pipelineState.Get(),			// PipelineState : 파이프라인 상태 객체(PSO)
		IID_PPV_ARGS(&m_commandList)
	));

	// CreateCommandList를 사용해서 
	// 커맨드리스트는 생성하면 OPEN상태이기 때문에
	// Close() 메서드를 호출하여 닫아야 합니다.
	ThrowIfFailed(m_commandList->Close());
}


/// <summary>
/// 업로드 힙의 데이터를 디폴트 힙으로 복사합니다.
/// 
/// </summary>
void GOERenderer::CopyUploadHeapToDefault()
{
	GD::GetInstance().WaitForFence(GD::GetInstance().m_fenceValue); // GPU가 이전 작업을 끝낼 때까지 기다립니다.

	// CopyBufferRegion() 메서드를 사용하여 업로드 힙의 데이터를 디폴트 힙으로 복사합니다.
	// 1. 커맨드 할당자와 커맨드 리스트 초기화
	// 이전에 기록된 GPU 작업(커맨드 리스트)이 끝났으니, 새롭게 명령을 기록할 수 있도록 할당자(Allocator)를 리셋합니다.
	m_commandAllocator->Reset();
	// 커맨드 리스트(실제 명령 기록 객체)를 리셋하고, 새 명령을 이 할당자에, 지정한 파이프라인 상태(m_pipelineState)로 기록하겠다고 선언.
	m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());

	// 모델 리소스의 메쉬 리소스를 순회하며 업로드 힙에서 디폴트 힙으로 복사합니다.
	// 큐브는 모델을 로드해서 그리는게 아니야
	for (const auto& meshResource : m_meshResourceMap)
	{
		m_commandList->CopyBufferRegion(
			meshResource.second->GetVBDefault(), 0,	// Dest
			meshResource.second->GetVBUpload(), 0,	// Src
			meshResource.second->GetVBSize()				// Size
		);
		// (3) 상태변환: 복사에서 VertexBuffer로 전환
		auto vsBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			meshResource.second->GetVBDefault(),	// pResource
			D3D12_RESOURCE_STATE_COPY_DEST,		// StateBefore
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER // StateAfter
		);
		m_commandList->ResourceBarrier(1, &vsBarrier);

		// 인덱스 버퍼도 동일한 방식으로 복사합니다.
		m_commandList->CopyBufferRegion(
			meshResource.second->GetIBDefault(), 0,
			meshResource.second->GetIBUpload(), 0,
			meshResource.second->GetIBSize());

		// 5. 상태변환
		auto ibBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
			meshResource.second->GetIBDefault(),	// pResource
			D3D12_RESOURCE_STATE_COPY_DEST,		// StateBefore
			D3D12_RESOURCE_STATE_INDEX_BUFFER	// StateAfter
		);

		m_commandList->ResourceBarrier(1, &ibBarrier);

	}

	m_commandList->Close();

	ID3D12CommandList* lists[] = { m_commandList.Get() };
	GD::GetInstance().m_commandQueue->ExecuteCommandLists(1, lists);
	GD::GetInstance().m_fenceValue++;
	GD::GetInstance().SignalFence(GD::GetInstance().m_fenceValue);
}

/// <summary>
/// imgui를 위한 디스크립터 힙 생성
/// </summary>
void GOERenderer::CreateImguiDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // CBV/SRV/UAV용
	desc.NumDescriptors = 64;               // 보통 ImGui는 1~2면 충분
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // 반드시 shader visible!
	ThrowIfFailed(GD::GetInstance().m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_imguiDescriptorHeap)));
}

void GOERenderer::LoadTexture(std::string filepath)
{
	// 파일경로를 통해 텍스처를 로드합니다.
	WIC_FLAGS wicFlags = WIC_FLAGS_NONE;
	TexMetadata metadata = {};
	ScratchImage data;

	HRESULT hr = DirectX::LoadFromWICFile(
		std::wstring(filepath.begin(), filepath.end()).c_str(),
		wicFlags, &metadata, data, nullptr);

	ThrowIfFailed(hr);

	/// 해셔와 관련된 내용은 
	/// 전용 클래스로 대체될 것이기 때문에 
	/// 굳이 임시변수로 만들어둔다.
	std::unique_ptr<TextureResource> textureResource
		= std::make_unique<TextureResource>(filepath,
			GOE::FileManager::GetHash(filepath));

	ComPtr<ID3D12Resource> textureDefault = nullptr;
	ComPtr<ID3D12Resource> textureUpload = nullptr;
	ComPtr<ID3D12DescriptorHeap> textureheap = {};
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle;

	// ScratchImage로부터 얻은 메타데이터로 리소스 속성을 정의합니다.
	D3D12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		metadata.format,
		metadata.width,
		metadata.height,
		static_cast<UINT16>(metadata.arraySize),
		static_cast<UINT16>(metadata.mipLevels));

	// --- 2. 디폴트 힙 생성 ---
	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	hr = GD::GetInstance().m_device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, // 데이터를 복사 받을 상태로 생성
		nullptr,
		IID_PPV_ARGS(&textureDefault)); // 멤버 변수 m_texture에 저장

	if (FAILED(hr))
	{
		return;
	}

	// 3. 업로드 힙 생성
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(textureDefault.Get(), 0, 1);
	CD3DX12_RESOURCE_DESC uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	hr = GD::GetInstance().m_device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&uploadBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&textureUpload)); // 멤버 변수 m_textureUploadHeap에 저장

	if (FAILED(hr))
	{
		return;
	}

	// --- 4. ScratchImage 데이터를 GPU 리소스로 복사하도록 명령 기록 ---
	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = data.GetPixels();	// ScratchImage의 픽셀 데이터
	subresourceData.RowPitch = data.GetImage(0, 0, 0)->rowPitch; // 한 줄의 바이트 크기
	subresourceData.SlicePitch = data.GetImage(0, 0, 0)->slicePitch; // 전체 이미지의 바이트 크기


	m_commandAllocator->Reset();
	// 커맨드 리스트(실제 명령 기록 객체)를 리셋하고, 새 명령을 이 할당자에, 지정한 파이프라인 상태(m_pipelineState)로 기록하겠다고 선언.
	m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());

	// 내부적으로 map과 unmap을 호출합니다.
	// 커맨드 리스트에 복사 명령을 기록합니다.
	UpdateSubresources(m_commandList.Get(), textureDefault.Get(), textureUpload.Get(), 0, 0, 1, &subresourceData);

	// --- 5. 텍스처 리소스 상태를 셰이더에서 읽을 수 있도록 변경 ---
	CD3DX12_RESOURCE_BARRIER barrier =
		CD3DX12_RESOURCE_BARRIER::Transition(
			textureDefault.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	m_commandList->ResourceBarrier(1, &barrier);

	// 디스크립터 힙 생성
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(GD::GetInstance().m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&textureheap)));

	// --- 6. 셰이더 리소스 뷰(SRV) 생성 ---
	// SRV를 생성할 디스크립터 힙의 핸들을 가져옵니다 (m_srvHeap은 미리 생성되어 있어야 함).
	srvHandle = textureheap->GetCPUDescriptorHandleForHeapStart();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	GD::GetInstance().m_device->CreateShaderResourceView(textureDefault.Get(), &srvDesc, srvHandle);

	textureResource.get()->SetTextureDefault(std::move(textureDefault));
	textureResource.get()->SetTextureUpload(std::move(textureUpload));
	textureResource.get()->SetTextureHeap(std::move(textureheap));
	textureResource.get()->SetSRVHandle(srvHandle);
	m_textureResourceMap[textureResource.get()->GetID()] = textureResource.get();
	m_textureResources.push_back(std::move(textureResource));


	// --- 7. 커맨드 리스트 실행 및 동기화 ---
	m_commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	GD::GetInstance().m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// GPU 작업이 완료될 때까지 기다립니다 (Fence 사용).
	GD::GetInstance().m_fenceValue++;
	GD::GetInstance().SignalFence(GD::GetInstance().m_fenceValue);
}

void GOERenderer::CreateAllMeshResources(const std::unordered_map<std::size_t, std::unique_ptr<Mesh>>& core_meshes)
{
	for (const auto& mesh : core_meshes)
	{
		// 메쉬리소스생성하고 추가한다.	
		m_meshResources.push_back(std::make_unique<MeshResource>(mesh.second.get()->GetName(), mesh.first));
		m_meshResourceMap[mesh.first] = m_meshResources.back().get();

		// 메쉬데이터를 가져옴
		Graphics::MeshData meshData(mesh.second.get()->GetMeshData());

		// 메쉬데이터를 리소스로 변환해서 방금 추가한 메쉬리소스에 추가
		CreateMeshResource(m_meshResourceMap[mesh.first], meshData);		

		// 추가된 메쉬리소스에  modelID와 meshIndex를 설정한다.
		// 모델 id 도 넣어야 한다.
		m_meshResourceMap[mesh.first]->SetMeshIndex(mesh.second.get()->GetMeshIndex());
		m_meshResourceMap[mesh.first]->SetModelID(mesh.second.get()->GetModelID());
	}
}

void GOERenderer::CreateOneMeshResource(const Mesh* core_mesh)
{
	// 메쉬리소스생성하고 추가한다.		
	m_meshResources.push_back(std::make_unique<MeshResource>(core_mesh->GetName(), core_mesh->GetID()));
	
	m_meshResourceMap[core_mesh->GetID()] = m_meshResources.back().get();

	// 메쉬데이터를 가져옴
	Graphics::MeshData meshData(core_mesh->GetMeshData());

	// 메쉬데이터를 리소스로 변환해서 방금 추가한 메쉬리소스에 추가
	CreateMeshResource(m_meshResourceMap[core_mesh->GetID()], meshData);

	// 추가된 메쉬리소스에  modelID와 meshIndex를 설정한다.
	// 모델 id 도 넣어야 한다.
	m_meshResourceMap[core_mesh->GetID()]->SetMeshIndex(core_mesh->GetMeshIndex());
	m_meshResourceMap[core_mesh->GetID()]->SetModelID(core_mesh->GetModelID());
}

/// <summary>
/// 메쉬데이터를 메쉬리소스를 채워주는 함수
/// </summary>
/// <param name="mesh_resource">랜더러의 메쉬리소스</param>
/// <param name="mesh_data">메쉬를 구성하는 메쉬데이터</param>
void GOERenderer::CreateMeshResource(MeshResource* mesh_resource, Graphics::MeshData& mesh_data)
{
	CreateVBResource(mesh_resource, mesh_data, D3D12_RESOURCE_STATE_GENERIC_READ);
	CreateIBResource(mesh_resource, mesh_data, D3D12_RESOURCE_STATE_GENERIC_READ);
	CreateCBResource(mesh_resource, mesh_data, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void GOERenderer::CreateVBResource(MeshResource* mesh_resource, const Graphics::MeshData& mesh_data, const D3D12_RESOURCE_STATES& state)
{
	/// 채워야 할 리소스
	size_t vertexBufferSize = sizeof(Graphics::Vertex) * mesh_data.vertices.size();
	ComPtr<ID3D12Resource> vertexBufferDefault = nullptr;
	ComPtr<ID3D12Resource> vertexBufferUpload = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

	// 1. 디폴트 힙 리소스 생성
	D3D12_HEAP_PROPERTIES defaultHeapProps = {};
	defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = vertexBufferSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;


	ThrowIfFailed(GD::GetInstance().m_device->CreateCommittedResource(
		&defaultHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, // 일단 복사 대상으로 생성
		nullptr,
		IID_PPV_ARGS(&vertexBufferDefault)
	));

	// D3D12_HEAP_PROPERTIES
	// : 힙의 속성을 정의하는 구조체입니다.
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD; // 리소스를 할당할 힙의 속성을 정의합니다. 

	// D3D12_RESOURCE_DESC
	// : 리소스의 속성을 정의하는 구조체입니다.
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;// 버퍼 리소스입니다.
	resDesc.Alignment = 0;								// 정렬은 0으로 설정합니다.(자동설정됨)
	resDesc.Width = vertexBufferSize;					// 버퍼라면 바이트 크기, 텍스쳐면 x축 픽셀 수
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

	ThrowIfFailed(GD::GetInstance().m_device->CreateCommittedResource(
		&heapProps,				// 힙 속성
		D3D12_HEAP_FLAG_NONE,	// 힙 플래그, 일반적으로 D3D12_HEAP_FLAG_NONE 사용
		&resDesc,				// 리소스 설명
		state, // 리소스 생성 직후의 상태
		nullptr,				// 최적화된 클리어 값 포인터(텍스처, RTV, DSV 등만 해당) 일반 버퍼는 nullptr
		IID_PPV_ARGS(&vertexBufferUpload))); // 반환될 인터페이스의 ID
	mesh_resource->SetVBUpload(vertexBufferUpload.Get()); // 업로드 힙 리소스 설정
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
	ThrowIfFailed(vertexBufferUpload->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	// memcpy() 함수를 사용하여 정점 데이터를 업로드 힙에 복사합니다.
	memcpy(pVertexDataBegin, mesh_data.vertices.data(), vertexBufferSize);
	// Unmap() 메서드는 업로드 힙의 매핑을 해제합니다.
	vertexBufferUpload->Unmap(0, nullptr);

	// D3D12_VERTEX_BUFFER_VIEW
	// : 정점 버퍼 뷰를 정의하는 구조체입니다.
	// 이후 DrawCall 시 이 정보를 넘김
	// 이 뷰는 GPU가 정점 데이터를 읽을 때 사용됩니다.
	vertexBufferView.BufferLocation = vertexBufferDefault->GetGPUVirtualAddress();	// GPU에서 읽을 정점버퍼 시작 주소, 정점 버퍼의 GPU 가상 주소
	vertexBufferView.StrideInBytes = sizeof(Graphics::Vertex);		// 정점버퍼 전체 크기(바이트 단위)
	vertexBufferView.SizeInBytes = vertexBufferSize;	// 정점 하나당 크기(바이트 단위)

	mesh_resource->SetVBSize(vertexBufferSize); // 정점 버퍼 크기 설정
	mesh_resource->SetVBDefault(vertexBufferDefault.Get()); // 디폴트 힙 리소스 설정
	mesh_resource->SetVBUpload(vertexBufferUpload.Get()); // 업로드 힙 리소스 설정
	mesh_resource->SetVBView(vertexBufferView); // 정점 버퍼 뷰 설정
}

void GOERenderer::CreateIBResource(MeshResource* mesh_resource, const Graphics::MeshData& mesh_data, const D3D12_RESOURCE_STATES& state)
{
	UINT64 indexBufferSize = sizeof(UINT32) * mesh_data.indices.size();
	ComPtr<ID3D12Resource> indexBufferDefault = nullptr;
	ComPtr<ID3D12Resource> indexBufferUpload = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};

	// 1. Default Heap (GPU)
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = indexBufferSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(GD::GetInstance().m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&indexBufferDefault)
	));

	// 2. Upload Heap (CPU)
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

	ThrowIfFailed(GD::GetInstance().m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		state,
		nullptr,
		IID_PPV_ARGS(&indexBufferUpload)
	));

	// 3. 데이터 복사
	UINT8* pIndexDataBegin;
	D3D12_RANGE readRange = { 0, 0 };
	ThrowIfFailed(indexBufferUpload->Map(0, &readRange, reinterpret_cast<void**>(&pIndexDataBegin)));

	memcpy(pIndexDataBegin, mesh_data.indices.data(), indexBufferSize);
	indexBufferUpload->Unmap(0, nullptr);

	// 6. 인덱스버퍼 뷰 생성
	indexBufferView.BufferLocation = indexBufferDefault->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = indexBufferSize;

	mesh_resource->Setm_IndexCount(mesh_data.indices.size());
	mesh_resource->SetIBSize(indexBufferSize); // 인덱스 버퍼 크기 설정
	mesh_resource->SetIBDefault(indexBufferDefault.Get()); // 디폴트 힙 리소스 설정
	mesh_resource->SetIBUpload(indexBufferUpload.Get()); // 업로드 힙 리소스 설정
	mesh_resource->SetIBView(indexBufferView); // 인덱스 버퍼 뷰 설정
}

void GOERenderer::CreateCBResource(MeshResource* mesh_resource, const Graphics::MeshData& mesh_data, const D3D12_RESOURCE_STATES& state)
{
	/// 여기부턴 본매트릭스를 위한 cb생성구간이다.
	ComPtr<ID3D12Resource> constantBuffer = {};
	ComPtr<ID3D12DescriptorHeap> CBVHeap = {};
	D3D12_CPU_DESCRIPTOR_HANDLE CBVHandle = {};

	// CBV디스크립터힙 heapProps
	D3D12_HEAP_PROPERTIES matrixheapProps = {};
	matrixheapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	matrixheapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	matrixheapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	matrixheapProps.CreationNodeMask = 0;
	matrixheapProps.VisibleNodeMask = 0;

	// 리소스 description
	D3D12_RESOURCE_DESC MatrixcbDesc = {};
	MatrixcbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	MatrixcbDesc.Alignment = 0;
	MatrixcbDesc.Width = sizeof(XMFLOAT4X4) * 128; // 최소 256바이트(행렬 64 + 패딩)
	MatrixcbDesc.Height = 1;
	MatrixcbDesc.DepthOrArraySize = 1;
	MatrixcbDesc.MipLevels = 1;
	MatrixcbDesc.Format = DXGI_FORMAT_UNKNOWN;
	MatrixcbDesc.SampleDesc.Count = 1;
	MatrixcbDesc.SampleDesc.Quality = 0;
	MatrixcbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	MatrixcbDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// CB 리소스생성
	HRESULT hr = GD::GetInstance().m_device->CreateCommittedResource(
		&matrixheapProps,
		D3D12_HEAP_FLAG_NONE,
		&MatrixcbDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constantBuffer)
	);
	ThrowIfFailed(hr);

	// CBV 디스크립터 힙 생성
	D3D12_DESCRIPTOR_HEAP_DESC MatrixheapDescCBV = {};
	MatrixheapDescCBV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // Constant Buffer View, Shader Resource View, Unordered Access View
	MatrixheapDescCBV.NumDescriptors = 1; // CBV 하나만 사용
	MatrixheapDescCBV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // 셰이더에서 접근 가능하도록 설정
	MatrixheapDescCBV.NodeMask = 0; // 멀티 GPU 시스템에서 사용할 노드 마스크, 단일 GPU 시스템에서는 1로 설정
	GD::GetInstance().m_device->CreateDescriptorHeap(&MatrixheapDescCBV, IID_PPV_ARGS(&CBVHeap));

	D3D12_CONSTANT_BUFFER_VIEW_DESC MatrixCBVDesc = {};
	// CBV 디스크립터 생성
	MatrixCBVDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress(); // CB 리소스의 GPU 가상 주소
	MatrixCBVDesc.SizeInBytes = ((sizeof(XMFLOAT4X4) * 128) + 255) & ~255; // CBV는 256바이트 정렬이 필요하므로, 크기를 256바이트로 올림 처리

	CBVHandle = CBVHeap->GetCPUDescriptorHandleForHeapStart();
	GD::GetInstance().m_device->CreateConstantBufferView(&MatrixCBVDesc, CBVHandle);

	/// 여기 잘 채우면 될듯? 
	/// 이미 float4x4를 채워놨으니까 안해도될듯
	XMFLOAT4X4 boneMatrix[128];
	// 1. 실제 boneOffset 데이터를 복사합니다.
	// vector의 data() 포인터를 사용해 메모리를 직접 복사하는 것이 효율적입니다.
	size_t offsetCount = mesh_data.boneOffsets.size();
	for (int i = 0; i < offsetCount; ++i)
	{
		boneMatrix[i] = mesh_data.boneOffsets[i];
	}
	for (int i = mesh_data.boneOffsets.size(); i < 128; ++i)
	{
		DirectX::XMStoreFloat4x4(&boneMatrix[i], /*XMMatrixTranspose(*/DirectX::XMMatrixIdentity());
	}

	void* pBoneData = nullptr;
	D3D12_RANGE boneReadRange = { 0, 0 };
	ThrowIfFailed(constantBuffer->Map(0, &boneReadRange, &pBoneData));
	memcpy(pBoneData, boneMatrix, sizeof(boneMatrix));
	constantBuffer->Unmap(0, nullptr);

	mesh_resource->SetCB(constantBuffer.Get()); // 업로드 힙 리소스 설정
	mesh_resource->SetCBVHeap(CBVHeap.Get()); // Constant Buffer View 디스크립터 힙 설정
	mesh_resource->SetCBVHandle(std::move(CBVHandle)); // Constant Buffer View 디스크립터 핸들 설정

}

void GOERenderer::CreateRenderObjectCBResource(RenderObject* render_object, const D3D12_RESOURCE_STATES& state)
{
	ComPtr<ID3D12Resource> constantBuffer = {};
	ComPtr<ID3D12DescriptorHeap> CBVHeap = {};
	D3D12_CPU_DESCRIPTOR_HANDLE CBVHandle = {};

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
	HRESULT hr = GD::GetInstance().m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&cbDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constantBuffer)
	);
	ThrowIfFailed(hr);

	// CBV 디스크립터 힙 생성
	D3D12_DESCRIPTOR_HEAP_DESC heapDescCBV = {};
	heapDescCBV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // Constant Buffer View, Shader Resource View, Unordered Access View
	heapDescCBV.NumDescriptors = 1; // CBV 하나만 사용
	heapDescCBV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // 셰이더에서 접근 가능하도록 설정
	heapDescCBV.NodeMask = 0; // 멀티 GPU 시스템에서 사용할 노드 마스크, 단일 GPU 시스템에서는 1로 설정
	GD::GetInstance().m_device->CreateDescriptorHeap(&heapDescCBV, IID_PPV_ARGS(&CBVHeap));

	D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc = {};
	// CBV 디스크립터 생성
	CBVDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress(); // CB 리소스의 GPU 가상 주소
	CBVDesc.SizeInBytes = (sizeof(Graphics::CB) + 255) & ~255; // CBV는 256바이트 정렬이 필요하므로, 크기를 256바이트로 올림 처리

	CBVHandle = CBVHeap->GetCPUDescriptorHandleForHeapStart();
	GD::GetInstance().m_device->CreateConstantBufferView(&CBVDesc, CBVHandle);

	Graphics::CB cbData = {};

	DirectX::XMStoreFloat4x4(&cbData.world, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&cbData.viewProjection, DirectX::XMMatrixIdentity());

	void* pData = nullptr;
	D3D12_RANGE readRange = { 0, 0 };
	ThrowIfFailed(constantBuffer->Map(0, &readRange, &pData));
	memcpy(pData, &cbData, sizeof(Graphics::CB));
	constantBuffer->Unmap(0, nullptr);

	render_object->SetCB(constantBuffer.Get()); // 업로드 힙 리소스 설정
	render_object->SetCBVHeap(CBVHeap.Get()); // Constant Buffer View 디스크립터 힙 설정
	render_object->SetCBVHandle(std::move(CBVHandle)); // Constant Buffer View 디스크립터 핸들 설정

	/// 여기부턴 본매트릭스를 위한 cb생성구간이다.
	ComPtr<ID3D12Resource> boneBuffer = {};
	ComPtr<ID3D12DescriptorHeap> boneCBVHeap = {};
	D3D12_CPU_DESCRIPTOR_HANDLE boneCBVHandle = {};

	// CBV디스크립터힙 heapProps
	D3D12_HEAP_PROPERTIES matrixheapProps = {};
	matrixheapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	matrixheapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	matrixheapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	matrixheapProps.CreationNodeMask = 0;
	matrixheapProps.VisibleNodeMask = 0;

	// 리소스 description
	D3D12_RESOURCE_DESC MatrixcbDesc = {};
	MatrixcbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	MatrixcbDesc.Alignment = 0;
	MatrixcbDesc.Width = sizeof(XMFLOAT4X4) * 128; // 최소 256바이트(행렬 64 + 패딩)
	MatrixcbDesc.Height = 1;
	MatrixcbDesc.DepthOrArraySize = 1;
	MatrixcbDesc.MipLevels = 1;
	MatrixcbDesc.Format = DXGI_FORMAT_UNKNOWN;
	MatrixcbDesc.SampleDesc.Count = 1;
	MatrixcbDesc.SampleDesc.Quality = 0;
	MatrixcbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	MatrixcbDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// CB 리소스생성
	hr = GD::GetInstance().m_device->CreateCommittedResource(
		&matrixheapProps,
		D3D12_HEAP_FLAG_NONE,
		&MatrixcbDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&boneBuffer)
	);
	ThrowIfFailed(hr);

	// CBV 디스크립터 힙 생성
	D3D12_DESCRIPTOR_HEAP_DESC MatrixheapDescCBV = {};
	MatrixheapDescCBV.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // Constant Buffer View, Shader Resource View, Unordered Access View
	MatrixheapDescCBV.NumDescriptors = 1; // CBV 하나만 사용
	MatrixheapDescCBV.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // 셰이더에서 접근 가능하도록 설정
	MatrixheapDescCBV.NodeMask = 0; // 멀티 GPU 시스템에서 사용할 노드 마스크, 단일 GPU 시스템에서는 1로 설정
	GD::GetInstance().m_device->CreateDescriptorHeap(&MatrixheapDescCBV, IID_PPV_ARGS(&boneCBVHeap));

	D3D12_CONSTANT_BUFFER_VIEW_DESC MatrixCBVDesc = {};
	// CBV 디스크립터 생성
	MatrixCBVDesc.BufferLocation = boneBuffer->GetGPUVirtualAddress(); // CB 리소스의 GPU 가상 주소
	MatrixCBVDesc.SizeInBytes = ((sizeof(XMFLOAT4X4) * 128) + 255) & ~255; // CBV는 256바이트 정렬이 필요하므로, 크기를 256바이트로 올림 처리

	boneCBVHandle = boneCBVHeap->GetCPUDescriptorHandleForHeapStart();
	GD::GetInstance().m_device->CreateConstantBufferView(&MatrixCBVDesc, boneCBVHandle);

	XMFLOAT4X4 boneMatrix[128];
	for (int i = 0; i < 128; ++i)
	{
		DirectX::XMStoreFloat4x4(&boneMatrix[i], DirectX::XMMatrixIdentity());
	}

	void* pBoneData = nullptr;
	D3D12_RANGE boneReadRange = { 0, 0 };
	ThrowIfFailed(boneBuffer->Map(0, &boneReadRange, &pBoneData));
	//memcpy(pBoneData, &pBoneData, sizeof(Graphics::Matrix4x4)*128);
	// 생성해둔 boneMatrix 배열의 데이터를 복사해야 합니다.
	memcpy(pBoneData, boneMatrix, sizeof(boneMatrix));
	boneBuffer->Unmap(0, nullptr);

	render_object->SetCBBoneMatrix(boneBuffer.Get()); // 업로드 힙 리소스 설정
	render_object->SetCBVoneMatrixHeap(boneCBVHeap.Get()); // Constant Buffer View 디스크립터 힙 설정
	render_object->SetCBVoneMatrixHandle(std::move(boneCBVHandle)); // Constant Buffer View 디스크립터 핸들 설정



}