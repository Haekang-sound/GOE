#pragma once
namespace Graphics
{
	enum ResourceState
	{
		LOADING = 0,
		READY
	};

	class RenderResource
	{

	public:
		RenderResource() = default;
		virtual ~RenderResource();

	public:
		inline Graphics::ResourceState GetState() const { return m_state; }

	public:
		inline void SetState(const Graphics::ResourceState state) { m_state = state; }

	protected:
		Graphics::ResourceState m_state = Graphics::ResourceState::LOADING;
	};
}
