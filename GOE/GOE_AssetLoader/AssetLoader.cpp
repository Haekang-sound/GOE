#include "AssetLoader_pch.h"
#include "AssetLoader.h"
#include <iostream> // 에러 출력을 위해 추가

// 메인 로드 함수
bool AssetLoader::LoadModelFromFile(const std::string& filePath, MeshData& outMeshData)
{
	// 이전 데이터를 모두 비웁니다.
	outMeshData.vertices.clear();
	outMeshData.indices.clear();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath,
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace);

	if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
	{
		std::cerr << "Assimp 로드 오류: " << importer.GetErrorString() << std::endl;
		return false;
	}

	// 루트 노드부터 재귀적으로 처리 시작. 초기 변환 행렬은 단위 행렬입니다.
	aiMatrix4x4 identity;
	ProcessNode(scene->mRootNode, scene, identity, outMeshData);

	return !outMeshData.vertices.empty();
}

// 노드를 재귀적으로 처리하여 변환을 누적하고 메쉬를 병합하는 함수
void AssetLoader::ProcessNode(aiNode* node, const aiScene* scene, const aiMatrix4x4& parentTransform, MeshData& outMeshData)
{
	// 부모로부터 받은 변환과 현재 노드의 변환을 곱하여 최종 변환 행렬을 계산합니다.
	aiMatrix4x4 currentTransform = parentTransform * node->mTransformation;

	// 현재 노드에 속한 모든 메쉬를 처리합니다.
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(mesh, scene, currentTransform, outMeshData);
	}

	// 모든 자식 노드에 대해 재귀 호출을 수행합니다.
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene, currentTransform, outMeshData);
	}
}

// 하나의 메쉬를 처리하여 outMeshData에 데이터를 추가하는 함수
void AssetLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4& transform, MeshData& outMeshData)
{
	// 인덱스를 추가할 때 참조할 시작 정점의 위치(오프셋)를 저장합니다.
	unsigned int baseVertex = static_cast<unsigned int>(outMeshData.vertices.size());

	// 1. 정점 데이터 추출 및 변환
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		VertexK vertex;

		// 위치(Position)를 변환 행렬로 변환합니다.
		aiVector3D pos = transform * mesh->mVertices[i];
		vertex.position[0] = pos.x;
		vertex.position[1] = pos.y;
		vertex.position[2] = pos.z;

		
		outMeshData.vertices.push_back(vertex);
	}

	// 2. 인덱스 데이터 추출 및 오프셋 적용
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			// 저장해둔 baseVertex 만큼 오프셋을 더해 전체 버퍼에 맞는 인덱스로 만듭니다.
			outMeshData.indices.push_back(face.mIndices[j] + baseVertex);
		}
	}
}