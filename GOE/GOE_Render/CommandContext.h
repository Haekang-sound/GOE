#pragma once
#include <d3d12.h>
#include <wrl.h>

namespace Graphics
{
	struct RenderContext;
	class CommandContext
	{
	public:
		CommandContext() = default;
		~CommandContext();

		void Initialize(RenderContext* renderContext);

	public:
		void CreateCommandAllocator();
		void CreateCommandList();

	public:
		ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;
		ComPtr<ID3D12GraphicsCommandList> m_commandList = nullptr;

		RenderContext* m_renderContext = nullptr;
	};
}
