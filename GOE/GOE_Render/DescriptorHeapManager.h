#pragma once
#include "RenderManager.h"

namespace Graphics
{
	struct RenderContext;

	class DescriptorHeapManager : public RenderManager
	{
	public:
		DescriptorHeapManager() = default;
		~DescriptorHeapManager();

	public:
		void Initialize(RenderContext* renderContext);
		int Allocate(UINT count, D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle);

	public:
		
		ID3D12DescriptorHeap* GetSRVHeap() const { return m_srvHeap.Get(); }
		UINT GetSRVDescriptorSize() const { return m_srvDescriptorSize; }

	private:
		ComPtr<ID3D12DescriptorHeap> m_srvHeap = nullptr; // SRV용 디스크립터 힙
		UINT m_srvDescriptorSize = 0; // SRV 디스크립터 크기
		UINT m_capacity = 2048; // 최대 SRV 디스크립터 수
		UINT m_currentOffset = 0; // 현재 할당된 디스크립터 오프셋
	
	};
}
