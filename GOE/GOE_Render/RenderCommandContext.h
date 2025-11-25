#pragma once
#include "CommandContext.h"
#include "RenderContext.h"
namespace Graphics
{
	/// <summary>
	/// render전용 커맨드 컨텍스트
	/// 
	/// </summary>
	class RenderCommandContext : public CommandContext
	{
	public: 
		~RenderCommandContext();
		void Initialize(RenderContext* renderContext) override;
		void Execute() override; 
	
	public:
		void TransitionToPresent(ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState);

	protected:
		void CreateCommandAllocator() override;
		void CreateCommandList() override;
	};
}
