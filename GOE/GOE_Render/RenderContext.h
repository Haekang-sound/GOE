#pragma once

namespace Graphics
{
	class GraphicsDevice;
	class SwapChain;
	class PSOManager;
	class CommandContext;
	class UIManager;
	class ResourceManager;

	/// <summary>
	/// 랜더러의 주요 매니저들을 모아놓은 컨텍스트
	/// </summary>
	struct RenderContext
	{
		GraphicsDevice* m_graphicsDevice;
		SwapChain* m_swapChain;
		PSOManager* m_PSOManager;
		CommandContext* m_commandContext;
		UIManager* m_UIManager;
		ResourceManager* m_resourceManager;
	};
}

