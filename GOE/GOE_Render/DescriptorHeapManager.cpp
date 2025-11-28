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
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // 셰이더에서 접근가능
	ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // 셰이더에서 접근가능
	ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_dynamicHeap)));

	m_srvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_currentOffset = 0;
	m_currentDynimicHeapOffset = 0;

}

/// <summary>
/// 리소스 핸들을 입력받고
/// 디스크립터 힙에서 디스크립터를 할당하여
/// 디스크립터 핸들을 반환한다.
/// (디스크립터 힙의 용량을 초과하면 -1을 반환한다.)
/// 
/// </summary>
/// <param name="count">디스크립터 갯수</param>
/// <param name="outCpuHandle">cpu handle</param>
/// <param name="outGpuHandle">gpu handle</param>
/// <returns>할당위치</returns>
int Graphics::DescriptorHeapManager::Allocate(UINT count, D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle)
{
	// 현재 오프셋이 용량을 초과하는지 확인
	if (m_currentOffset + count > m_capacity)
	{
		return -1; // 할당 실패
	}

	// 시작주소
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();

	// 오프셋 적용
	cpuHandle.ptr += static_cast<SIZE_T>(m_currentOffset) * m_srvDescriptorSize;

	// 오프셋이 적용된 핸들 반환
	if (outCpuHandle) *outCpuHandle = cpuHandle;

	int allocatedOffset = static_cast<int>(m_currentOffset);
	
	// 오프셋 증가
	m_currentOffset += count;

	// 할당된 오프셋 반환
	return allocatedOffset;
}

/// <summary>
/// 동적인 디스크립터를 할당하는 함수
/// 
/// </summary>
/// <param name="count">디스크립터 갯수</param>
/// <param name="outCpuHandle">cpu handle</param>
/// <param name="outGpuHandle">gpu handle</param>
/// <returns>할당위치</returns>
int Graphics::DescriptorHeapManager::AllocateDynamic(UINT count, D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle)
{
	// 현재 오프셋이 용량을 초과하는지 확인
	if (m_currentDynimicHeapOffset + count > m_capacity)
	{
		return -1; // 할당 실패
	}

	// 시작주소
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_dynamicHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_dynamicHeap->GetGPUDescriptorHandleForHeapStart();

	// 오프셋 적용
	cpuHandle.ptr += static_cast<SIZE_T>(m_currentDynimicHeapOffset) * m_srvDescriptorSize;
	gpuHandle.ptr += static_cast<SIZE_T>(m_currentDynimicHeapOffset) * m_srvDescriptorSize;

	// 오프셋이 적용된 핸들 반환
	if (outCpuHandle) *outCpuHandle = cpuHandle;
	if (outGpuHandle) *outGpuHandle = gpuHandle;

	int allocatedOffset = static_cast<int>(m_currentDynimicHeapOffset);

	// 오프셋 증가
	m_currentDynimicHeapOffset += count;

	// 할당된 오프셋 반환
	return allocatedOffset;
}

/// <summary>
/// 정적 디스크립터 힙에서 동적 힙으로 복사합니다.
/// </summary>
/// <param name="srcCpuHandle"></param>
/// <param name="outGpuHandle"></param>
void Graphics::DescriptorHeapManager::CopyToDynamicHeap(D3D12_CPU_DESCRIPTOR_HANDLE srcCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle)
{
	D3D12_CPU_DESCRIPTOR_HANDLE destCpuHandle = {};
	int offset = AllocateDynamic(1, &destCpuHandle, outGpuHandle);
	
	if (offset != -1)
	{
		m_renderContext->m_graphicsDevice->m_device->CopyDescriptorsSimple(
			1,
			destCpuHandle,
			srcCpuHandle,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
}

/// <summary>
/// 다이나믹 힙 오프셋을 리셋합니다.
/// </summary>
void Graphics::DescriptorHeapManager::ResetDynamicHeap()
{
	m_currentDynimicHeapOffset = 0;
}
