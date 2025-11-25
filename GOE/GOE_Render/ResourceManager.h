#pragma once
#include <vector>
#include <memory>
#include <unordered_map>

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
		void UpdateResourceStates();

		void LoadTexture(std::string filepath);
		void CreateMeshResource(const Mesh* core_mesh);

	public:
		// ... Get 함수들은 그대로 ...
		inline const std::unordered_map<size_t, std::shared_ptr<MeshResource>>& GetMeshResourceMap() const
		{
			return m_meshResourceMap;
		}
		inline const std::unordered_map<size_t, std::shared_ptr<TextureResource>>& GetTextureResourceMap() const
		{
			return m_textureResourceMap;
		}
		inline std::shared_ptr<TextureResource> GetTextureResource(size_t texture_id)
		{
			auto it = m_textureResourceMap.find(texture_id);
			if (it != m_textureResourceMap.end())
			{
				return it->second;
			}
			return nullptr;
		}
		inline std::shared_ptr<MeshResource> GetMeshResource(size_t mesh_id)
		{
			auto it = m_meshResourceMap.find(mesh_id);
			if (it != m_meshResourceMap.end())
			{
				return it->second; 
			}
			return nullptr;
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

		// '로딩 중'인 리소스와 완료 목표 펜스 값을 저장하는 목록
		std::vector<std::pair<UINT64, std::weak_ptr<TextureResource>>> m_loadingTextures;
		std::vector<std::pair<UINT64, std::weak_ptr<MeshResource>>> m_loadingMeshes;

		std::vector<ComPtr<ID3D12Resource>> uploadBuffers; // 임시 리소스 보관용 벡터

	private:
		RenderContext* m_renderContext = nullptr;
	};
}


