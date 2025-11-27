#include "Renderer_pch.h"
#include "CopyCommandContext.h"

Graphics::CopyCommandContext::~CopyCommandContext() = default;

void Graphics::CopyCommandContext::Initialize(RenderContext* renderContext)
{
	m_renderContext = renderContext;
	CreateCommandAllocator();
	CreateCommandList();
	m_commandList->Close();
}

void Graphics::CopyCommandContext::Execute()
{
	const auto device = m_renderContext->m_graphicsDevice;
	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	device->m_copyCmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	device->SignalCopyFence();
	m_committedFaenceValue = device->GetCopyFenceValue();
}

void Graphics::CopyCommandContext::Reset()
{
	const auto device = m_renderContext->m_graphicsDevice;

	device->WaitForCopyFenceValue(m_committedFaenceValue);
	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
}

void Graphics::CopyCommandContext::TransitionToCommon(ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState)
{
	ResourceBarrier(
		resource,
		currentState,
		D3D12_RESOURCE_STATE_COMMON);
}

void Graphics::CopyCommandContext::CreateCommandAllocator()
{
	const auto device = m_renderContext->m_graphicsDevice;
	ThrowIfFailed(device->m_device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_COPY,
		IID_PPV_ARGS(&m_commandAllocator)));
}

void Graphics::CopyCommandContext::CreateCommandList()
{
	const auto device = m_renderContext->m_graphicsDevice;
	ThrowIfFailed(device->m_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_COPY,
		m_commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&m_commandList)));
	int a = 3;
}
