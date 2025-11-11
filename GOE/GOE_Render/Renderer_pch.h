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
#include <DirectXMath.h>

// 랜더러 핵심기능
#include "ID3DRenderer.h"
#include "GraphicsDevice.h"

// 랜더러 기본자료형
#include "D3DMath.h"
// 너무 기니까 줄인다.
using GD = Graphics::GraphicsDevice;
using namespace DirectX;

struct RenderObjectData
{
	std::string name;
	size_t id = 0;
	size_t meshID = 0;
	size_t meshIndex = 0;
	size_t modelID = 0;
	size_t textureID = 0;
	GOE::Matrix4x4 localTM = GOE::Matrix4x4::Identity();
	GOE::Matrix4x4 boneTM[128];
	bool isVisible = false;
	bool isAnimated = false;

public:
	RenderObjectData()
	{
		for (int i = 0; i < 128; ++i)
		{
			boneTM[i] = GOE::Matrix4x4::Identity();
		}
	}

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

