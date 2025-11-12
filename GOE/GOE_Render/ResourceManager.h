#pragma once

class TextureResource;

namespace Graphics
{
	class CommandContext;

	class ResourceManager
	{
	public:
		ResourceManager() = default;
		~ResourceManager();

		void Initialize(RenderContext* renderContext);

		void LoadTexture(std::string filepath, CommandContext& commandContext);

		std::vector<std::unique_ptr<TextureResource>> m_textureResources;
		std::unordered_map<size_t, TextureResource*> m_textureResourceMap;

		RenderContext* m_renderContext = nullptr;
	};
}


