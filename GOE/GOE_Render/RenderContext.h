#pragma once

namespace Graphics
{
	class GraphicsDevice;
	class SwapChain;
	class PSOManager;
	class RenderCommandContext;
	class CopyCommandContext;
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
		UIManager* m_UIManager;
		ResourceManager* m_resourceManager;
		// command
		RenderCommandContext* m_commandContext;
		CopyCommandContext* m_copyCommandContext;
	};
}

