#pragma once
#include <d3d12.h>
#include "RenderManager.h"
namespace Graphics
{
	struct RenderContext;
	class PSOManager : public RenderManager
	{
	public:
		PSOManager() = default;
		~PSOManager();
	public: 
		void Initialize(RenderContext* renderContext) override;

	public:
		ComPtr<ID3D12RootSignature> GetRootSignature() const { return m_rootSignature; }
		ComPtr<ID3D12PipelineState> GetPipelineState() const { return m_pipelineState; }
		ComPtr<ID3DBlob> GetVertexShader() const { return m_vertexShader; }
		ComPtr<ID3DBlob> GetPixelShader() const { return m_pixelShader; }
		D3D12_INPUT_ELEMENT_DESC* GetInputElementDescs() { return m_inputElementDescs; }

	public: 
		void SetInputElementDesc(
			const UINT& index,
			const char* semanticName,
			const UINT& semanticIndex,
			const DXGI_FORMAT& format,
			const UINT& inputSlot,
			const UINT& alignedByteOffset,
			const D3D12_INPUT_CLASSIFICATION& inputSlotClass,
			const UINT& instanceDataStepRate);
		void SetInputElementDescs(
			const D3D12_INPUT_ELEMENT_DESC* inputElementDescs,
			const UINT& count);
		void SetVertexShader(ComPtr<ID3DBlob> vertexShader) { m_vertexShader = vertexShader; }
		void SetPixelShader(ComPtr<ID3DBlob> pixelShader) { m_pixelShader = pixelShader; }
		void SetRootSignature(ComPtr<ID3D12RootSignature> rootSignature) { m_rootSignature = rootSignature; }
		void SetPipelineState(ComPtr<ID3D12PipelineState> pipelineState) { m_pipelineState = pipelineState; }
	
	public:
		HRESULT CompileShaderFromFile(const WCHAR* fileName, const WCHAR* entryPoint, const WCHAR* targetProfile, ID3DBlob** ppShaderBlob);

	protected:
		void CreateRootSignature();
		void CompileShaders();
		void CreatePipelineState();

	public:
		static constexpr UINT m_inputElementDescCount = 6;

	protected:
		ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;
		ComPtr<ID3DBlob> m_vertexShader = nullptr;
		ComPtr<ID3DBlob> m_pixelShader = nullptr;
		D3D12_INPUT_ELEMENT_DESC m_inputElementDescs[m_inputElementDescCount] = {};
		ComPtr<ID3D12PipelineState> m_pipelineState = nullptr;
	};
}


