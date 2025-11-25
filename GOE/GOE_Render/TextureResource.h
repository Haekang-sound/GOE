#pragma once
#include "RenderResource.h"

class TextureResource : public Graphics::RenderResource
{
public:
	TextureResource(std::string name, size_t id)
		: m_name(name), m_id(id){}

	~TextureResource();

public:
	inline const std::string& GetName() const { return m_name; }
	inline const size_t& GetID() const { return m_id; }
	inline ID3D12Resource* GetTextureDefault() const { return textureDefault.Get(); }
	inline ID3D12Resource* GetTextureUpload() const { return textureUpload.Get(); }
	inline ID3D12DescriptorHeap* GetTextureHeap() const { return textureheap.Get(); }
	inline const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRVHandle() const { return srvHandle; }

public:
	inline void SetTextureDefault(ComPtr<ID3D12Resource>&& resource) { textureDefault = std::move(resource); }
	inline void SetTextureUpload(ComPtr<ID3D12Resource>&& resource) { textureUpload = std::move(resource); }
	// 디스크립터 힙의 관에 관한 이해는 부족하지만 일단 이렇게 처리
	inline void SetTextureHeap(ComPtr<ID3D12DescriptorHeap>&& heap) { textureheap = std::move(heap); }
	inline void SetSRVHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& handle) { srvHandle = handle; }


private:
	std::string m_name; // 텍스처 이름
	const size_t m_id = 0; // 텍스처 ID
	
private:
	ComPtr<ID3D12Resource> textureDefault = nullptr;
	ComPtr<ID3D12Resource> textureUpload = nullptr;
	ComPtr<ID3D12DescriptorHeap> textureheap = {};
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = {};
};

