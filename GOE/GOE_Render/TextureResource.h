#pragma once
class TextureResource
{
public:
	TextureResource(std::string name, size_t id)
		: m_name(name), m_id(id){}

	~TextureResource();


private:
	std::string m_name; // 텍스처 이름
	const size_t m_id = 0; // 텍스처 ID
	
private:
	ComPtr<ID3D12Resource> m_textureBuffer = {};
	ComPtr<ID3D12DescriptorHeap> m_textureHeap = {};
	D3D12_CPU_DESCRIPTOR_HANDLE m_textureHandle = {};
};

