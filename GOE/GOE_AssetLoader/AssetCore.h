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
	void LoadModel(const std::string& filePath, GOE::MeshData& outMeshData);

public:
	std::unique_ptr<AssetLoader> m_assetLoader = nullptr;
};