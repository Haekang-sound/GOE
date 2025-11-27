#pragma once
namespace Graphics
{
	struct RenderContext;
	class RenderManager
	{
	public:
		RenderManager() = default;
		virtual ~RenderManager();
	public:
		virtual void Initialize(RenderContext* renderContext) = 0;

	public: 
		RenderContext* GetRenderContext() const { return m_renderContext; }

	public:
		void SetRenderContext(RenderContext* renderContext) { m_renderContext = renderContext; }

	protected:
		RenderContext* m_renderContext = nullptr;
	};
}


