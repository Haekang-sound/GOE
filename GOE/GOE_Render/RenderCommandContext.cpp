#include "Renderer_pch.h"
#include "RenderCommandContext.h"
#include <d3dx12/d3dx12.h>
#include "PSOManager.h"

Graphics::RenderCommandContext::~RenderCommandContext() = default;

void Graphics::RenderCommandContext::Initialize(RenderContext* renderContext)
{
	m_renderContext = renderContext;
	CreateCommandAllocator();
	CreateCommandList();
	m_commandList->Close();
}

/// <summary>
/// 커맨드리스트를 제출하는 함수
/// </summary>
void Graphics::RenderCommandContext::Execute()
{
	const auto device = m_renderContext->m_graphicsDevice;
	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	device->m_renderCmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	device->SignalRenderFence();
	m_committedFaenceValue = device->GetRenderFenceValue();
}

void Graphics::RenderCommandContext::TransitionToPresent(ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState)
{
	ResourceBarrier(
		resource,
		currentState,
		D3D12_RESOURCE_STATE_PRESENT); 
}

void Graphics::RenderCommandContext::CreateCommandAllocator()
{
	const auto device = m_renderContext->m_graphicsDevice;
	ThrowIfFailed(device->m_device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&m_commandAllocator)));
}

void Graphics::RenderCommandContext::CreateCommandList()
{
	const auto device = m_renderContext->m_graphicsDevice;
	ThrowIfFailed(device->m_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&m_commandList)));
}
