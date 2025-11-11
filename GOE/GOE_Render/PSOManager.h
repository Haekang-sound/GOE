#pragma once
#include <d3d12.h>
namespace Graphics
{
	class PSOManager
	{
	public:
		PSOManager() = default;
		~PSOManager();

		void Initialize();

		void CreateRootSignature();
		void CompileShaders();
		void CreatePipelineState();
		HRESULT CompileShaderFromFile(const WCHAR* fileName, const WCHAR* entryPoint, const WCHAR* targetProfile, ID3DBlob** ppShaderBlob);

		ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
		ComPtr<ID3DBlob> m_vertexShader = nullptr;
		ComPtr<ID3DBlob> m_pixelShader = nullptr;
		D3D12_INPUT_ELEMENT_DESC m_inputElementDescs[6] = {};
		ComPtr<ID3D12PipelineState> m_pipelineState = nullptr;
	};
}


