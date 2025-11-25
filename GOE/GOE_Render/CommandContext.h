#pragma once
#include <d3d12.h>
#include <wrl.h>

struct ID3D12PipelineState;
namespace Graphics
{
	struct RenderContext;
	class CommandContext
	{
	public:
		CommandContext() = default;
		virtual ~CommandContext();

		virtual void Initialize(RenderContext* renderContext) = 0;
		virtual void Reset();
		virtual void Reset(ID3D12PipelineState* pipe);
		virtual void Execute() = 0;

	public:
		ID3D12CommandAllocator* GetCommandAllocator() const { return m_commandAllocator.Get(); }
		ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }
		UINT64 GetCommittedFenceValue() const { return m_committedFaenceValue; }

	public:
		void SetCommittedFenceValue(UINT64 value) { m_committedFaenceValue = value; }

	public:
		void CopyResource(
			ID3D12Resource* destResource,
			ID3D12Resource* srcResource,
			UINT64 byteSize,
			D3D12_RESOURCE_STATES state);

	protected:
		void ResourceBarrier(
			ID3D12Resource* resource,
			D3D12_RESOURCE_STATES stateBefore,
			D3D12_RESOURCE_STATES stateAfter);
		virtual void CreateCommandAllocator() = 0;
		virtual void CreateCommandList() = 0;

	protected:
		ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;
		ComPtr<ID3D12GraphicsCommandList> m_commandList = nullptr;
		UINT64 m_committedFaenceValue = 0;
		RenderContext* m_renderContext = nullptr;
	};
}
