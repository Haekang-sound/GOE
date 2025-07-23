#pragma once  
///
/// STL의 컴파일 부담을 없애려면
/// 미리컴파일된 헤더를 사용하는게 좋을것
/// 같다고 판단해서 만들게된 pch 파일입니다.
/// 
/// ohk 2025.07.21
#include <../GOE_Core/Core_pch.h>

// directX
#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <d3dx12/d3dx12.h>
using namespace DirectX;

struct MVP  
{
	XMFLOAT4X4 mvp; // 64바이트(행렬), row-major/col-major는 HLSL에서 맞춰줌  
};  

struct Vertex  
{
	XMFLOAT3 position;  
	XMFLOAT4 color = {0,0,0,1};
};

struct UIInitInfo
{
	ID3D12Device* device = nullptr;
	ID3D12CommandQueue* commandQueue = nullptr;
	ID3D12DescriptorHeap* imguiDescriptorHeap = nullptr;
	int frameBufferCount = {};
};

struct UILoopInfo
{
	ID3D12DescriptorHeap* imguiDescriptorHeap = nullptr;
	ID3D12GraphicsCommandList* commandlist = nullptr;
	ID3D12Resource* rendertarget = nullptr;
};
