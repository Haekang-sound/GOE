#pragma once

#include <wrl.h>
struct ID3D12DescriptorHeap;
struct UIInitInfo;
struct UILoopInfo;
namespace Graphics
{
	struct RenderContext;

	/// <summary>
	/// ui관련 자료들을 관리하는 매니저
	/// </summary>
	class UIManager
	{
	public:
		UIManager();
		~UIManager();
		UIManager(const UIManager&) = delete;
		UIManager(UIManager&&) = delete;
		UIManager& operator=(const UIManager&) = delete;
		UIManager& operator=(UIManager&&) = delete;

	public:
		void Initialize(RenderContext* renderContext);
		void CreateImguiDescriptorHeap();
		UIInitInfo* GetUIInfo();
		UILoopInfo* GetUILoopInfo();

	public:
		ComPtr<ID3D12DescriptorHeap> m_imguiDescriptorHeap = nullptr;
		std::unique_ptr<UILoopInfo> m_UILoopInfo;
		std::unique_ptr<UIInitInfo> m_UIInitInfo;

		RenderContext* m_renderContext = nullptr;

	};
}

