#include "Renderer_pch.h"
#include "UIManager.h"
#include "CommandContext.h"
#include "SwapChain.h"

Graphics::UIManager::UIManager()
{
	m_UIInitInfo = std::make_unique<UIInitInfo>();
	m_UILoopInfo = std::make_unique <UILoopInfo>();
}

Graphics::UIManager::~UIManager() = default;

void Graphics::UIManager::Initialize(RenderContext* renderContext)
{
	m_renderContext = renderContext;
	CreateImguiDescriptorHeap();
}
/// <summary>
/// imgui를 위한 디스크립터 힙 생성
/// 리소스매니저가 만들어지면 개선되어야 할지도 모른다.
/// </summary>
void Graphics::UIManager::CreateImguiDescriptorHeap()
{
	const auto device = m_renderContext->m_graphicsDevice;
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // CBV/SRV/UAV용
	desc.NumDescriptors = 64;               // 보통 ImGui는 1~2면 충분
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // 반드시 shader visible!
	ThrowIfFailed(device->m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_imguiDescriptorHeap)));
}

UIInitInfo* Graphics::UIManager::GetUIInfo()
{
	const auto device = m_renderContext->m_graphicsDevice;
	const auto swapChain = m_renderContext->m_swapChain;
	m_UIInitInfo.get()->commandQueue = device->m_commandQueue.Get();
	m_UIInitInfo.get()->device = device->m_device.Get();
	m_UIInitInfo.get()->frameBufferCount = swapChain->m_frameBufferCount;
	m_UIInitInfo.get()->imguiDescriptorHeap = m_imguiDescriptorHeap.Get();
	return m_UIInitInfo.get();
}
UILoopInfo* Graphics::UIManager::GetUILoopInfo()
{
	const auto commandList = m_renderContext->m_commandContext->m_commandList;
	const auto swapChain = m_renderContext->m_swapChain;

	m_UILoopInfo.get()->commandlist = commandList.Get();
	m_UILoopInfo.get()->imguiDescriptorHeap = m_imguiDescriptorHeap.Get();
	m_UILoopInfo.get()->rendertarget = swapChain->m_renderTargets[swapChain->m_frameIndex].Get();
	return m_UILoopInfo.get();
}

