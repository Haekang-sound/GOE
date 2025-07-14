#pragma once  

#include <d3d12.h>
#include <d3dx12/d3dx12.h>

#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include <string>
#include <wrl.h>
#include <shellapi.h>

#include <comdef.h>
#include <windows.h>
#include <stdexcept>
#include <functional>
using namespace DirectX;
// GOETypes 헤더 파일에 필요한 구조체 및 타입 정의  

struct MVP  
{
	XMFLOAT4X4 mvp; // 64바이트(행렬), row-major/col-major는 HLSL에서 맞춰줌  
};  

struct Vertex  
{
	DirectX::XMFLOAT3 position;  
    DirectX::XMFLOAT4 color;  
};

inline void ThrowIfFailed(const HRESULT& hr)
{
	if (FAILED(hr))
	{
		throw std::runtime_error("HRESULT failed!");
	}
}

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
