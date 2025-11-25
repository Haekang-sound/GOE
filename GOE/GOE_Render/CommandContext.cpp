#include "Renderer_pch.h"
#include "CommandContext.h"

#include<d3dx12/d3dx12.h>

#include "PSOManager.h"

Graphics::CommandContext::~CommandContext() = default;

void Graphics::CommandContext::Reset()
{
	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
}
void Graphics::CommandContext::Reset(ID3D12PipelineState* pipe)
{
	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), pipe));
}
/// <summary>
/// 버퍼간 데이터를 교환한다.
/// 보통 업로드와 디폴트 데이터를 교환할때 사용한다.
/// </summary>
/// <param name="destResource">디폴트 버퍼</param>
/// <param name="srcResource">업로드 버퍼</param>
/// <param name="byteSize">버퍼 사이즈</param>
void Graphics::CommandContext::CopyResource(ID3D12Resource* destResource, ID3D12Resource* srcResource, UINT64 byteSize, D3D12_RESOURCE_STATES state)
{
	m_commandList->CopyResource(destResource, srcResource);
}

void Graphics::CommandContext::ResourceBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		resource,	// pResource
		stateBefore,
		stateAfter // StateAfter
	);
	m_commandList->ResourceBarrier(1, &barrier);
}
