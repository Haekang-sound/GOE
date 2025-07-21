#include "AssetLoader_pch.h"
#include "AssetLoader.h"


// 모델 파일을 로드하는 함수
bool AssetLoader::LoadModelFromFile(const std::string& filePath, MeshData& outMeshData)
{
	Assimp::Importer importer;

	// 모델 파일을 읽고 기본적인 후처리를 적용합니다.
	// aiProcess_Triangulate: 모든 면을 삼각형으로 만듭니다 (필수).
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
	{
		std::cerr << "Assimp 로드 오류: " << importer.GetErrorString() << std::endl;
		return false;
	}

	// 간단한 예시로, 첫 번째 메시만 사용합니다.
	// 실제 엔진에서는 모든 메시를 순회하며 처리해야 합니다.
	if (scene->mNumMeshes > 0)
	{
		aiMesh* mesh = scene->mMeshes[0];

		// 정점 데이터 추출
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			VertexK vertex;
			vertex.position[0] = mesh->mVertices[i].x;
			vertex.position[1] = mesh->mVertices[i].y;
			vertex.position[2] = mesh->mVertices[i].z;
			outMeshData.vertices.push_back(vertex);
		}

		// 인덱스 데이터 추출
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				outMeshData.indices.push_back(face.mIndices[j]);
			}
		}
		return true;
	}
	return false;
}

