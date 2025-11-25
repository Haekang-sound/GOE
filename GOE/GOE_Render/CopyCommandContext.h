#pragma once
#include "CommandContext.h"
#include <d3d12.h>
#include <wrl.h>
/// <summary>
/// copy전용 커맨드 컨텍스트
/// 
/// </summary>
namespace Graphics
{
	struct RenderContext;

	class CopyCommandContext : public CommandContext
	{
	public:
		~CopyCommandContext();

	public:
		void Initialize(RenderContext* renderContext) override;
		void Execute() override;
		void Reset() override;

	public:
		void TransitionToCommon(ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState);


	protected:
		void CreateCommandAllocator() override;
		void CreateCommandList() override;
	};

}