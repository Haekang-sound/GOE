#include "AssetLoader_pch.h"
#include "AssetLoader.h"
#include "../GOE_Core/Commons.h"

// 메인 로드 함수
bool AssetLoader::LoadModelFromFile(const std::string& filePath, MeshData& outMeshData)
{
	// Create an instance of the Importer class
	Assimp::Importer importer;
	// DirectX 호환을 위한 플래그 설정
	const unsigned int dxFlags =		// dx설정
		aiProcess_MakeLeftHanded |		// 1. 왼손 좌표계로 변환
		aiProcess_FlipUVs |				// 2. 텍스처 V좌표 뒤집기
		aiProcess_Triangulate |			// (권장) 모든 면을 삼각형으로 분할
		aiProcess_CalcTangentSpace |	// (권장) 탄젠트와 바이탄젠트 계산
		aiProcess_JoinIdenticalVertices |// 동일한 정점 병합
		aiProcess_SortByPType;			// 3. 프리미티브 타입별로 정렬

	// 자체제작 io를 쓰기엔 부족한게 많은듯? 
	//importer.SetIOHandler(new AssimpIOSystem());

	const aiScene* scene = importer.ReadFile(filePath, dxFlags);
		

	// If the import failed, report it
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		// 에러 처리: 씬 로딩 실패
		return false;
	}

	// Now we can access the file's contents.
	// Process the root node recursively
	if (scene->HasMeshes())
	{
		// 씬에 메쉬가 있는 경우, 각 메쉬를 처리합니다.
		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[i];
			processMesh(mesh, scene, outMeshData);
		}
	}

	// We're done. Everything will be cleaned up by the importer destructor
	return true;
}

void AssetLoader::processMesh(aiMesh* mesh, const aiScene* scene, MeshData& outMeshData)
{
	// 정점(Vertex) 데이터 추출
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		VertexK vertex;
		// 위치 (Position)
		// Assimp의 aiVector3D는 3D 벡터를 나타내는 구조체입니다.
		// aiVector3D는 Assimp에서 제공하는 벡터 타입으로, x, y, z 좌표를 포함합니다.
		aiVector3D position = mesh->mVertices[i];
		vertex.position[0] = mesh->mVertices[i].x;
		vertex.position[1] = mesh->mVertices[i].y;
		vertex.position[2] = mesh->mVertices[i].z;
		outMeshData.vertices.push_back(vertex);

	}

	// 인덱스(Index) 데이터 추출
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			unsigned int index = face.mIndices[j];
			outMeshData.indices.push_back(index);
		}
	}

}
