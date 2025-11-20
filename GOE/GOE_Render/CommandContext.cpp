#include "Renderer_pch.h"
#include "CommandContext.h"

#include<d3dx12/d3dx12.h>

#include "PSOManager.h"

Graphics::CommandContext::~CommandContext() = default;

void Graphics::CommandContext::Initialize(RenderContext* renderContext)
{
	m_renderContext = renderContext;
	const auto pipelineState = m_renderContext->m_PSOManager->m_pipelineState.Get();
	CreateCommandAllocator();
	CreateCommandList();
}
/// <summary>
/// 커맨드 얼로케이터를 생성합니다.
/// 
/// </summary>
/// <returns></returns>
void Graphics::CommandContext::CreateCommandAllocator()
{
	const auto device = m_renderContext->m_graphicsDevice;
	ThrowIfFailed(device->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

/// <summary>
/// 커맨드리스트를 생성합니다.
/// 
/// </summary>
/// <returns></returns>
void Graphics::CommandContext::CreateCommandList()
{
	const auto device = m_renderContext->m_graphicsDevice;
	const auto pipelineState = m_renderContext->m_PSOManager->m_pipelineState.Get();
	// D3D12_COMMAND_LIST_TYPE_DIRECT
	// : 이 타입은 GPU에 직접 명령을 보내는 커맨드 리스트를 생성합니다.
	ThrowIfFailed(device->m_device->CreateCommandList(
		0,								// NODMASK
		D3D12_COMMAND_LIST_TYPE_DIRECT, // TYPE : 커맨드리스트 타입(무엇을 기록할지 종류) 지정
		m_commandAllocator.Get(),		// CommandAllocator : 커맨드리스트와 1:1 매핑은 아님! (여러 번 재사용 가능)
		pipelineState,			// PipelineState : 파이프라인 상태 객체(PSO)
		IID_PPV_ARGS(&m_commandList)
	));

	// CreateCommandList를 사용해서 
	// 커맨드리스트는 생성하면 OPEN상태이기 때문에
	// Close() 메서드를 호출하여 닫아야 합니다.
	ThrowIfFailed(m_commandList->Close());
}
/// <summary>
/// 버퍼간 데이터를 교환한다.
/// 보통 업로드와 디폴트 데이터를 교환할때 사용한다.
/// </summary>
/// <param name="destResource">디폴트 버퍼</param>
/// <param name="srcResource">업로드 버퍼</param>
/// <param name="byteSize">버퍼 사이즈</param>
UINT64 Graphics::CommandContext::CopyResource(ID3D12Resource* destResource, ID3D12Resource* srcResource, UINT64 byteSize, D3D12_RESOURCE_STATES state)
{
	const auto resourceManager = m_renderContext->m_resourceManager;
	const auto device = m_renderContext->m_graphicsDevice;
	const auto PSOManager = m_renderContext->m_PSOManager;

	device->WaitForFence(); // GPU가 이전 작업을 끝낼 때까지 기다립니다.

	// CopyBufferRegion() 메서드를 사용하여 업로드 힙의 데이터를 디폴트 힙으로 복사합니다.
	// 1. 커맨드 할당자와 커맨드 리스트 초기화
	// 이전에 기록된 GPU 작업(커맨드 리스트)이 끝났으니, 새롭게 명령을 기록할 수 있도록 할당자(Allocator)를 리셋합니다.
	m_commandAllocator->Reset();
	// 커맨드 리스트(실제 명령 기록 객체)를 리셋하고, 새 명령을 이 할당자에, 지정한 파이프라인 상태(m_pipelineState)로 기록하겠다고 선언.
	m_commandList->Reset(m_commandAllocator.Get(), PSOManager->m_pipelineState.Get());

	m_commandList->CopyBufferRegion(
		destResource, 0,	// Dest
		srcResource, 0,	// Src
		byteSize);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		destResource,	// pResource
		D3D12_RESOURCE_STATE_COPY_DEST,
		state // StateAfter
	);
	m_commandList->ResourceBarrier(1, &barrier);
	m_commandList->Close();

	ID3D12CommandList* lists[] = { m_commandList.Get() };
	device->m_commandQueue->ExecuteCommandLists(1, lists);
	device->SignalFence();
	return device->m_fenceValue;
}
