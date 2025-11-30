#include "Renderer_pch.h"
#include "GraphicsDevice.h"
#include "DescriptorHeapManager.h"
#include <d3dx12/d3dx12.h>

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

	// 공용 UploadBuffer초기화
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_uploadBufferCapacity);

	ThrowIfFailed(device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_uploadBuffer)));
	D3D12_RANGE readRange = { 0,0 };
	ThrowIfFailed(m_uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedBufferPtr)));
	m_currentUploadBufferOffset = 0;

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

D3D12_GPU_VIRTUAL_ADDRESS Graphics::DescriptorHeapManager::AllocateConstantBuffer(const void* data, size_t size)
{
	if (data == nullptr)
	{
		OutputDebugStringA("Error: AllocateConstantBuffer data is nullptr\n");
		return 0;
	}

	if (m_mappedBufferPtr == nullptr)
	{
		OutputDebugStringA("Error: m_mappedBufferPtr is nullptr\n");
		return 0;
	}

	size_t alinedSize = (size + 255) & ~255;

	if (m_currentUploadBufferOffset + size > m_uploadBufferCapacity) // size로 체크 (memcpy 안전 보장)
	{
		OutputDebugStringA("Error: Constant Buffer Overflow\n");
		return 0;
	}
	memcpy(m_mappedBufferPtr + m_currentUploadBufferOffset, data, size);
	D3D12_GPU_VIRTUAL_ADDRESS gpuAdress = m_uploadBuffer->GetGPUVirtualAddress() + m_currentUploadBufferOffset;
	m_currentUploadBufferOffset += static_cast<UINT>(alinedSize);

	return gpuAdress;
}

/// <summary>
/// 정적 디스크립터 힙에서 동적 힙으로 복사합니다.
/// 1. 원본 정적 디스크립터와 사용할 GPU핸들을 입력받습니다.
/// 2. 임시 CPU핸들과 입력받은 GPU핸들을 통해 동적 힙에서 디스크립터를 할당받습니다.
/// 3. 원본 정적디스크립터의 내용을 임시 CPU핸들을 통해 동적 힙으로 복사합니다.
/// 4. 임시CPU과 연동된 GPU핸들을 통해서 랜더링에 사용할 수 있습니다.
/// 
/// </summary>
/// <param name="srcCpuHandle">원본 디스크립터</param>
/// <param name="outGpuHandle">랜더에 사용할 디스크립터</param>
void Graphics::DescriptorHeapManager::CopyToDynamicHeap(D3D12_CPU_DESCRIPTOR_HANDLE srcCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle)
{
	// 임시변수
	D3D12_CPU_DESCRIPTOR_HANDLE destCpuHandle = {};

	// 다이나믹 힙으로부터 
	// 디스크립터 공간을 할당받는다.
	int offset = AllocateDynamic(1, &destCpuHandle, outGpuHandle);

	if (offset != -1)
	{
		// 원본 디스크립터를 다이나믹 힙으로 복사
		m_renderContext->m_graphicsDevice->m_device->CopyDescriptorsSimple(
			1,
			destCpuHandle,
			srcCpuHandle,
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
}

/// <summary>
/// 
/// 다이나믹 힙 오프셋을 리셋합니다.
/// 
/// </summary>
void Graphics::DescriptorHeapManager::ResetDynamicHeap()
{
	m_currentDynimicHeapOffset = 0;
}

void Graphics::DescriptorHeapManager::ResetCB()
{
	m_currentUploadBufferOffset = 0;
}
