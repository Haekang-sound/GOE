#pragma once

class TextureResource;
class MeshResource;

namespace Graphics
{
	class CommandContext;
	struct MeshData;
	struct RenderContext;

	class ResourceManager
	{
	public:
		ResourceManager() = default;
		~ResourceManager();

		void Initialize(RenderContext* renderContext);

		void LoadTexture(std::string filepath, CommandContext& commandContext);
		void CreateMeshResource(const Mesh* core_mesh);

	public:
		inline const std::unordered_map<size_t, std::shared_ptr<MeshResource>>& GetMeshResourceMap() const
		{
			return m_meshResourceMap; // 메쉬 리소스 맵 반환
		}
		inline const std::unordered_map<size_t, std::shared_ptr<TextureResource>>& GetTextureResourceMap() const
		{
			return m_textureResourceMap; // 텍스처 리소스 맵 반환
		}
		inline std::shared_ptr<TextureResource> GetTextureResource(size_t texture_id)
		{
			auto it = m_textureResourceMap.find(texture_id);
			if (it != m_textureResourceMap.end())
			{
				return it->second; // 찾았으면 텍스처 리소스 반환
			}
			return nullptr; // 못 찾았으면 nullptr 반환
		}
		inline std::shared_ptr<MeshResource> GetMeshResource(size_t mesh_id)
		{
			auto it = m_meshResourceMap.find(mesh_id);
			if (it != m_meshResourceMap.end())
			{
				return it->second; // 찾았으면 메쉬 리소스 반환
			}
			return nullptr; // 못 찾았으면 nullptr 반환
		}

		ComPtr<ID3D12Resource> CreateUploadBuffer(
			const void* initialData,
			size_t bufferSize);

		ComPtr<ID3D12Resource> CreateCBResource(
			const void* initialData,
			size_t bufferSize,
			const D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_GENERIC_READ);

		void CreateVBResource(
			MeshResource* mesh_resource,
			const Graphics::MeshData& mesh_data,
			const D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_GENERIC_READ);

		void CreateIBResource(
			MeshResource* mesh_resource,
			const Graphics::MeshData& mesh_data,
			const D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_GENERIC_READ);

	private:
		ComPtr<ID3D12DescriptorHeap> CBVHeap = {};
		D3D12_CPU_DESCRIPTOR_HANDLE CBVHandle = {};

	private:
		std::unordered_map<size_t, std::shared_ptr<TextureResource>> m_textureResourceMap;
		std::unordered_map<size_t, std::shared_ptr<MeshResource>> m_meshResourceMap;

	private:
		RenderContext* m_renderContext = nullptr;
	};
}


