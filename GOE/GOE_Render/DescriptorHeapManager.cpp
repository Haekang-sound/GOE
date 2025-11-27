#include "Renderer_pch.h"
#include "GraphicsDevice.h"
#include "DescriptorHeapManager.h"

Graphics::DescriptorHeapManager::~DescriptorHeapManager() = default;

void Graphics::DescriptorHeapManager::Initialize(RenderContext* renderContext)
{
	m_renderContext = renderContext;
	const auto device = m_renderContext->m_graphicsDevice->m_device;

	// SRV 디스크립터 힙 생성
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = m_capacity;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // 셰이더에서 접근가능

	ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
	// 디스크립터 크기 계산
	// 디스크립터 크기는 디바이스마다 다를 수 있으므로, 디바이스에서 가져와야 한다.
	m_srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_currentOffset = 0;

}

/// <summary>
/// 리소스 핸들을 입력받고
/// 디스크립터 힙에서 디스크립터를 할당하여
/// 디스크립터 핸들을 반환한다.
/// (디스크립터 힙의 용량을 초과하면 -1을 반환한다.)
/// 
/// </summary>
/// <param name="count"></param>
/// <param name="outCpuHandle"></param>
/// <param name="outGpuHandle"></param>
/// <returns></returns>
int Graphics::DescriptorHeapManager::Allocate(UINT count, D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle)
{
	// 현재 오프셋이 용량을 초과하는지 확인
	if (m_currentOffset + count > m_capacity)
	{
		return -1; // 할당 실패
	}

	// 시작주소
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_srvHeap->GetGPUDescriptorHandleForHeapStart();

	// 오프셋 적용
	cpuHandle.ptr += static_cast<SIZE_T>(m_currentOffset) * m_srvDescriptorSize;
	gpuHandle.ptr += static_cast<SIZE_T>(m_currentOffset) * m_srvDescriptorSize;

	// 오프셋이 적용된 핸들 반환
	if (outCpuHandle) *outCpuHandle = cpuHandle;
	if (outGpuHandle) *outGpuHandle = gpuHandle;

	int allocatedOffset = static_cast<int>(m_currentOffset);
	
	// 오프셋 증가
	m_currentOffset += count;

	// 할당된 오프셋 반환
	return allocatedOffset;
}
