#include "Renderer_pch.h"
#include "CommandContext.h"
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
