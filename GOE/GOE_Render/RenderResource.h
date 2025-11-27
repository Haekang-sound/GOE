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
		D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCpuHandle() const { return srvcpuHandle; }
		D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGpuHandle() const { return srvGpuHandle; }

	public:
		inline void SetState(const Graphics::ResourceState state) { m_state = state; }
		inline void SetSRVHandles(const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, const D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle)
		{
			srvcpuHandle = cpuHandle;
			srvGpuHandle = gpuHandle;
		}

	protected:
		Graphics::ResourceState m_state = Graphics::ResourceState::LOADING;

		D3D12_CPU_DESCRIPTOR_HANDLE srvcpuHandle = {};
		D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle = {};
	};
}
