#include <memory> 
#include <string>

namespace GOE
{
	struct MeshData;
}

class AssetLoader;

/// <summary>
/// 에셋을 불러오기위한 전초기지
/// 
/// ohk 2025.07.17
/// </summary>
class AssetCore
{
public:
	AssetCore();
	~AssetCore();

public:
	void CreateAssetLoader();
	bool LoadModel(const std::string& filePath);
	// view를 한번써보기
	bool LoadAnimation(const std::string_view filePath);

	const std::unordered_map<std::size_t, std::unique_ptr<Model>>& GetModels() const;
	const std::unordered_map<std::size_t, std::unique_ptr<Mesh>>& GetMeshes() const;
	const Mesh* GetMesh(size_t id);
	Model* GetModel(size_t id);
	Animation* GetAnimation(size_t id);
	
public:
	std::unique_ptr<AssetLoader> m_assetLoader = nullptr;
};