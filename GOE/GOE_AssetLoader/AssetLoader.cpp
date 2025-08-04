#include "AssetLoader_pch.h"
#include "AssetLoader.h"
#include "../GOE_Core/Commons.h"
#include "../GOE_Core/Model.h"
#include "../GOE_Core/Node.h"
#include "../GOE_Core/Mesh.h"

// 메인 로드 함수
bool AssetLoader::LoadModelFromFile(const std::string& filePath)
{
	// hasher 객체를 함수처럼 호출하여 filePath의 해시 값을 계산합니다.
	size_t pathHash = hasher(filePath);

	m_models[pathHash] = Model(pathHash); // 모델을 해시맵에 추가

	// Create an instance of the Importer class
	Assimp::Importer importer;

	// DirectX 호환을 위한 플래그 설정
	const unsigned int dxFlags =		// dx설정
		aiProcess_MakeLeftHanded |		// 1. 왼손 좌표계로 변환
		aiProcess_FlipUVs |				// 2. 텍스처 V좌표 뒤집기
		aiProcess_FlipWindingOrder |
		aiProcess_Triangulate |			// (권장) 모든 면을 삼각형으로 분할
		aiProcess_CalcTangentSpace |	// (권장) 탄젠트와 바이탄젠트 계산
		aiProcess_JoinIdenticalVertices |// 동일한 정점 병합
		aiProcess_SortByPType | 			// 3. 프리미티브 타입별로 정렬
		aiProcess_ValidateDataStructure |        // 로더의 출력을 검증
		aiProcess_ImproveCacheLocality |        // 출력 정점의 캐쉬위치를 개선
		aiProcess_RemoveRedundantMaterials |    // 중복된 매터리얼 제거
		aiProcess_GenUVCoords |                    // 구형, 원통형, 상자 및 평면 매핑을 적절한 UV로 변환
		aiProcess_TransformUVCoords |            // UV 변환 처리기 (스케일링, 변환...)
		aiProcess_FindInstances |                // 인스턴스된 매쉬를 검색하여 하나의 마스터에 대한 참조로 제거
		aiProcess_LimitBoneWeights |            // 정점당 뼈의 가중치를 최대 4개로 제한
		aiProcess_OptimizeMeshes |                // 가능한 경우 작은 매쉬를 조인
		aiProcess_GenSmoothNormals |            // 부드러운 노말벡터(법선벡터) 생성
		aiProcess_SplitLargeMeshes;           // 거대한 하나의 매쉬를 하위매쉬들로 분활(나눔)


	const aiScene* scene = importer.ReadFile(filePath, dxFlags);

	// If the import failed, report it
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		// 에러 처리: 씬 로딩 실패
		return false;
	}

	// 노드를 처리한다.
	if (scene->HasMeshes())
	{
		aiNode* rootNode = scene->mRootNode;
		rootNode->mName.C_Str(); // 노드 이름을 가져옵니다.
		m_models[pathHash].AddRootNode(ProcessNode(rootNode));

		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			// 현재 메쉬를 가져옵니다.
			aiMesh* mesh = scene->mMeshes[i];
			// 메쉬 데이터를 처리합니다.
			m_models[pathHash].AddMesh(ProcessMesh(mesh, scene));
		}
	}

	return true;
}

Node AssetLoader::ProcessNode(aiNode* node)
{
	/// 노드이름을 해쉬로 저장중-> 이걸로 충분한지는 의문
	Node currentNode = Node(node->mName.C_Str(), hasher(node->mName.C_Str()));
	// 노드의 자식 노드를 재귀적으로 처리합니다.
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		// 메쉬인덱스
		unsigned int meshIndex = node->mMeshes[i];
		currentNode.AddMeshIndex(meshIndex); // 현재 노드가 참조하는 메쉬 인덱스를 추가합니다.
		// 현재 노드의 자식 노드를 가져옵니다.
		aiNode* childNode = node->mChildren[i];
		// 자식 노드를 재귀적으로 처리합니다.
		currentNode.AddChild(ProcessNode(childNode));
	}

	return currentNode; // 현재 노드에 대한 처리가 끝났으므로 빈 노드를 반환합니다.
}

Mesh AssetLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	Mesh currentMesh(mesh->mName.C_Str(), hasher(mesh->mName.C_Str()));

	// 정점(Vertex) 데이터 추출
	GOE::MeshData meshData;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		GOE::Vertex vertex;
		aiVector3D position = mesh->mVertices[i];
		vertex.position[0] = mesh->mVertices[i].x;
		vertex.position[1] = mesh->mVertices[i].y;
		vertex.position[2] = mesh->mVertices[i].z;
		meshData.vertices.push_back(vertex);
	}

	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			meshData.indices.push_back(face.mIndices[j]);
		}
	}

	currentMesh.SetMeshData(std::move(meshData));

	return currentMesh;
}
