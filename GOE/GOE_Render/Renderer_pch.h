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



#include "ID3DRenderer.h"
#include "D3DMath.h"

using namespace DirectX;

struct RenderObjectData
{
	std::string name;
	size_t id;
	size_t meshID;
	size_t meshIndex;
	size_t modelID;
	size_t textureID;
	GOE::Matrix4x4 localTM;
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

