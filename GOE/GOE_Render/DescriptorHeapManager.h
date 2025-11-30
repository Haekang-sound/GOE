#pragma once
#include "RenderManager.h"

namespace Graphics
{
	struct RenderContext;
	/// <summary>
	/// 그래픽스 리소스들을 사용할 수 있게 가공, 저장합니다.
	/// 
	/// </summary>
	class DescriptorHeapManager : public RenderManager
	{
	public:
		DescriptorHeapManager() = default;
		~DescriptorHeapManager();

	public:
		void Initialize(RenderContext* renderContext);
		int Allocate(UINT count, D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle);
		int AllocateDynamic(UINT count, D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle);
		void CopyToDynamicHeap(D3D12_CPU_DESCRIPTOR_HANDLE srcCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle);
		void ResetDynamicHeap();
		void ResetCB();
		D3D12_GPU_VIRTUAL_ADDRESS AllocateConstantBuffer(const void* data, size_t size);

	public:
		ID3D12DescriptorHeap* GetSRVHeap() const { return m_srvHeap.Get(); }
		ID3D12DescriptorHeap* GetDynamicSRVHeap() const { return m_dynamicHeap.Get(); }
		UINT GetSRVDescriptorSize() const { return m_srvDescriptorSize; }

	private:
		UINT m_srvDescriptorSize = 0; // SRV 디스크립터 크기, 하드웨어기준으로 디스크립터 크기는 동일하기 때문에 하나만 저장
		UINT m_capacity = 2048; // 이것도 일단공유할까? 

		// 정적힙
		ComPtr<ID3D12DescriptorHeap> m_srvHeap = nullptr; // SRV용 디스크립터 힙
		UINT m_currentOffset = 0; // 현재 할당된 디스크립터 오프셋
		
		// 동적힙
		ComPtr<ID3D12DescriptorHeap> m_dynamicHeap = nullptr; // SRV용 디스크립터 힙
		UINT m_currentDynimicHeapOffset = 0; // 현재 할당된 디스크립터 오프셋

		// 공용 업로드 버퍼
		ComPtr<ID3D12Resource> m_uploadBuffer = nullptr;
		UINT8* m_mappedBufferPtr = nullptr;

		UINT m_uploadBufferCapacity = 64 * 1024 * 1024;
		UINT m_currentUploadBufferOffset = 0;
	
	};
}
