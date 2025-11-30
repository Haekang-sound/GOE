#include "Renderer_pch.h"
#include "PSOManager.h"
#include "dxcapi.h"

#include <d3dx12/d3dx12.h>

#include <filesystem>


Graphics::PSOManager::~PSOManager() = default;

void Graphics::PSOManager::Initialize(RenderContext* renderContext)
{
	m_renderContext = renderContext;
	CreateRootSignature();
	CompileShaders();
	CreatePipelineState();
}
void Graphics::PSOManager::SetInputElementDesc(const UINT& index, const char* semanticName, const UINT& semanticIndex, const DXGI_FORMAT& format, const UINT& inputSlot, const UINT& alignedByteOffset, const D3D12_INPUT_CLASSIFICATION& inputSlotClass, const UINT& instanceDataStepRate)
{
	m_inputElementDescs[index].SemanticName = semanticName;
	m_inputElementDescs[index].SemanticIndex = semanticIndex;
	m_inputElementDescs[index].Format = format;
	m_inputElementDescs[index].InputSlot = inputSlot;
	m_inputElementDescs[index].AlignedByteOffset = alignedByteOffset;
	m_inputElementDescs[index].InputSlotClass = inputSlotClass;
	m_inputElementDescs[index].InstanceDataStepRate = instanceDataStepRate;
}

void Graphics::PSOManager::SetInputElementDescs(const D3D12_INPUT_ELEMENT_DESC* inputElementDescs, const UINT& count)
{
	for (UINT i = 0; i < count; ++i)
	{
		m_inputElementDescs[i] = inputElementDescs[i];
	}
}
/// <summary>
/// 루트시그니처를 생성합니다.
/// 
/// 루트시그니처는 파이프라인 상태를 설정하는데 사용되며,
/// 입력 어셈블러 단계에서 사용되는 리소스의 바인딩을 정의합니다.
/// </summary>
/// <returns></returns>
void Graphics::PSOManager::CreateRootSignature()
{
	const auto device = m_renderContext->m_graphicsDevice;
	// 1. 배열 선언 (2개)
	CD3DX12_DESCRIPTOR_RANGE ranges[2];
	// 2. 초기화 (Init 함수 사용)
	// 파라미터 순서: Type, NumDescriptors, BaseShaderRegister, RegisterSpace(생략가능), Offset(생략가능)
	// range[0]: SRV, 1개, t0
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	// range[1]: CBV, 1개, b1 (BaseRegister가 1이므로)
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	D3D12_ROOT_PARAMETER rootParameters[4] = {}; // 총 4개의 파라미터 (CBV 3개 + Table 1개)
	// 파라미터 0: 월드/뷰/투영 CBV (b0) - RenderObject 소유
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].Descriptor.ShaderRegister = 0; // b0 레지스터
	rootParameters[0].Descriptor.RegisterSpace = 0;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // 모든 셰이더 단계에서 접근 가능

	// 파라미터 1: 본 변환 행렬 CBV (b1) - RenderObject 소유
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].Descriptor.ShaderRegister = 1;
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
	rootParameters[3].DescriptorTable.pDescriptorRanges = &ranges[0]; // 위에서 정의한 범위 사용
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
	ThrowIfFailed(device->m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
}


HRESULT Graphics::PSOManager::CompileShaderFromFile(
	const WCHAR* fileName,
	const WCHAR* entryPoint,
	const WCHAR* targetProfile,
	ID3DBlob** ppShaderBlob)
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

	DxcBuffer sourceBuffer = {};
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
void Graphics::PSOManager::CompileShaders()
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
void Graphics::PSOManager::CreatePipelineState()
{
	const auto device = m_renderContext->m_graphicsDevice;
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
	ThrowIfFailed(device->m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}
