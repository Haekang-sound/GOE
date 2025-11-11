#include "AssetLoader_pch.h"
#include "AssetLoader.h"

AssetLoader::~AssetLoader() = default; // 소멸자 구현

// 메인 로드 함수
bool AssetLoader::LoadModelFromFile(const std::string& filePath)
{
	// hasher 객체를 함수처럼 호출하여 filePath의 해시 값을 계산합니다.
	size_t pathHash = GOE::FileManager::GetHash(filePath);

	m_models[pathHash] = std::make_unique<Model>(pathHash); // 모델을 해시맵에 추가

	// Create an instance of the Importer class
	Assimp::Importer importer;

	// DirectX 호환을 위한 플래그 설정
	const unsigned int modelFlags =
		aiProcess_LimitBoneWeights |   // 최대 4개 본 제한
		aiProcess_MakeLeftHanded |
		aiProcess_FlipUVs |
		aiProcess_FlipWindingOrder |
		aiProcess_Triangulate |
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_ValidateDataStructure |
		aiProcess_ImproveCacheLocality |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_OptimizeMeshes |
		aiProcess_GenSmoothNormals |
		aiProcess_SplitLargeMeshes |
		aiProcess_PopulateArmatureData;


	const aiScene* scene = importer.ReadFile(filePath, modelFlags);

	// If the import failed, report it
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		// 에러 처리: 씬 로딩 실패
		return false;
	}

	// 노드를 처리한다.
	if (scene->HasMeshes())
	{
		// 여기서 노드를 트리형태로 저장하고
		aiNode* rootNode = scene->mRootNode;
		rootNode->mName.C_Str(); // 노드 이름을 가져옵니다.
		m_models[pathHash].get()->AddRootNode(ProcessNode(rootNode));
		// 트리로 저장된 노드들을 순회하면서 벡터에담으면서 인덱스를 부여한다.
		// 동시에 해쉬-노드 형태로 저장한다.
		Node* start = m_models[pathHash].get()->GetRootNode().get();

		///dfs를 사용해서 트리탐색
		std::vector<Node*> nodeStack;
		nodeStack.push_back(start);
		while (nodeStack.size() > 0)
		{
			Node* temp = nodeStack.back();
			nodeStack.pop_back();
			temp->SetModelID(m_models[pathHash].get()->GetID());
			temp->SetNodeIndex(m_models[pathHash].get()->GetNodeVector().size());
			m_models[pathHash].get()->AddNodeToVector(temp);
			m_models[pathHash].get()->AddNodeToMap(temp->GetID(), temp);

			for (int i = 0; i < temp->GetChildren().size(); ++i)
			{
				nodeStack.push_back(temp->GetChildren()[i].get());
			}
		}


		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			// 현재 메쉬를 가져옵니다.
			aiMesh* mesh = scene->mMeshes[i];

			// 메쉬 데이터를 처리합니다.
			/// 진정 메쉬정보는 메쉬랜더러가 갖는게 맞다. 모델은 메쉬와의 관계를 소유한다.
			m_meshes[GOE::FileManager::GetHash(mesh->mName.C_Str())] = ProcessMesh(mesh, scene);
			m_meshes[GOE::FileManager::GetHash(mesh->mName.C_Str())].get()->SetModelID(pathHash); // 메쉬에 모델 ID 설정
			m_meshes[GOE::FileManager::GetHash(mesh->mName.C_Str())].get()->SetMeshIndex(static_cast<std::size_t>(i)); // 메쉬 인덱스 설정
			m_models[pathHash].get()->AddMeshID(m_meshes[GOE::FileManager::GetHash(mesh->mName.C_Str())].get()->GetID());
			m_models[pathHash].get()->AddMeshToMap(m_meshes[GOE::FileManager::GetHash(mesh->mName.C_Str())].get()->GetID(),
				m_meshes[GOE::FileManager::GetHash(mesh->mName.C_Str())].get()->GetMeshIndex());
		}
	}

	return true;
}

bool AssetLoader::LoadAnimiationFromFile(const std::string& filePath)
{
	// Create an instance of the Importer class
	Assimp::Importer importer;

	const unsigned int animFlags =
		aiProcess_MakeLeftHanded |     // DX 좌표계 맞춤
		aiProcess_FlipUVs |            // UV 뒤집기 (필요 시)
		aiProcess_FlipWindingOrder |   // 인덱스 시계방향 뒤집기
		aiProcess_Triangulate |        // 삼각형화 (본 매핑용)
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_LimitBoneWeights |   // 정점당 본 4개 제한
		//aiProcess_GlobalScale |        // FBX/Collada 단위계 보정 -> 이거 잘못사용하면망함;
		aiProcess_ValidateDataStructure; // 데이터 구조 검증

	const aiScene* scene = importer.ReadFile(filePath, animFlags);

	// If the import failed, report it
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		// 에러 처리: 씬 로딩 실패
		return false;
	}


	/// 애니메이션 클래스를 채우고 보관한다.
	if (scene->HasAnimations())
	{
		for (int i = 0; i < scene->mNumAnimations; ++i)
		{
			aiAnimation* anim = scene->mAnimations[i];

			std::unique_ptr<Animation> a = std::make_unique<Animation>(anim->mName.C_Str(), GOE::FileManager::GetHash(anim->mName.C_Str()));

			a.get()->SetDuation(anim->mDuration);
			a.get()->SetTicksPerSecond(anim->mTicksPerSecond);

			for (int j = 0; j < anim->mNumChannels; ++j)
			{
				std::unique_ptr<BoneAnimation> boneAnim = std::make_unique<BoneAnimation>(anim->mChannels[j]->mNodeName.C_Str(), GOE::FileManager::GetHash(anim->mChannels[j]->mNodeName.C_Str()));
				int test = 3;
				for (int k = 0; k < anim->mChannels[j]->mNumPositionKeys; ++k)
				{
					auto posKey = anim->mChannels[j]->mPositionKeys[k];
					boneAnim.get()->AddPositions({ posKey.mTime, {
						posKey.mValue.x,
						posKey.mValue.y,
						posKey.mValue.z } });
				}

 				for (int k = 0; k < anim->mChannels[j]->mNumScalingKeys; ++k)
				{
					auto scaleKey = anim->mChannels[j]->mScalingKeys[k];
					boneAnim.get()->AddScales({ scaleKey.mTime, {
						scaleKey.mValue.x,
						scaleKey.mValue.y,
						scaleKey.mValue.z } });
				}

				for (int k = 0; k < anim->mChannels[j]->mNumRotationKeys; ++k)
				{
					auto rotKey = anim->mChannels[j]->mRotationKeys[k];
					boneAnim.get()->AddQuatanions(
						{ rotKey.mTime,
						{rotKey.mValue.x,
						rotKey.mValue.y,
						rotKey.mValue.z,
						rotKey.mValue.w } });
				}

				a.get()->AddBoneAnimation(move(boneAnim));
			}
			m_animations[GOE::FileManager::GetHash(anim->mName.C_Str())] = move(a);
		}

	}


	return true;
}

std::unique_ptr<Node> AssetLoader::ProcessNode(aiNode* node, Node* parent)
{
	/// 노드이름을 해쉬로 저장중-> 이걸로 충분한지는 의문
	std::unique_ptr<Node> currentNode = std::make_unique<Node>(node->mName.C_Str(), GOE::FileManager::GetHash(node->mName.C_Str()));

	currentNode.get()->SetLocalTM(aiMatrix4x4ToCoreMtrix(node->mTransformation));
	currentNode.get()->SetNodePosition(
		node->mTransformation.a4,
		node->mTransformation.b4,
		node->mTransformation.c4);
	currentNode.get()->SetParent(parent); // 부모노드 설정

	
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		unsigned int meshIndex = node->mMeshes[i];
		currentNode.get()->AddMeshIndex(meshIndex); // 현재 노드가 참조하는 메쉬 인덱스를 추가합니다.
	}

	// 2. 이 노드의 *자식 노드*들을 재귀적으로 처리합니다.
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		// 현재 노드의 자식 노드를 가져옵니다.
		aiNode* childNode = node->mChildren[i];
		// 자식 노드를 재귀적으로 처리합니다.
		currentNode.get()->AddChild(ProcessNode(childNode, currentNode.get()));
	}

	return currentNode; // 현재 노드에 대한 처리가 끝났으므로 빈 노드를 반환합니다.
}

std::unique_ptr<Mesh> AssetLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	std::unique_ptr<Mesh> currentMesh = std::make_unique<Mesh>(mesh->mName.C_Str(), GOE::FileManager::GetHash(mesh->mName.C_Str()));

	// 정점(Vertex) 데이터 추출
	std::unique_ptr<GOE::MeshData> meshData = std::make_unique<GOE::MeshData>();
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		meshData.get()->vertices.emplace_back();
		meshData.get()->vertices.back().position.x = mesh->mVertices[i].x;
		meshData.get()->vertices.back().position.y = mesh->mVertices[i].y;
		meshData.get()->vertices.back().position.z = mesh->mVertices[i].z;


		if (mesh->HasTextureCoords(0))
		{
			meshData.get()->vertices.back().uv.x = mesh->mTextureCoords[0][i].x; // UV 좌표 X
			meshData.get()->vertices.back().uv.y = mesh->mTextureCoords[0][i].y; // UV 좌표 Y
		}
		else
		{
			meshData.get()->vertices.back().uv.x = 0.0f; // UV 좌표 X
			meshData.get()->vertices.back().uv.y = 0.0f; // UV 좌표 Y
		}

		if (mesh->HasNormals())
		{
			meshData.get()->vertices.back().normal.x = mesh->mNormals[i].x;
			meshData.get()->vertices.back().normal.y = mesh->mNormals[i].y;
			meshData.get()->vertices.back().normal.z = mesh->mNormals[i].z;
		}
		else
		{
			meshData.get()->vertices.back().normal.x = 0.0f;
			meshData.get()->vertices.back().normal.y = 0.0f;
			meshData.get()->vertices.back().normal.z = 0.0f;
		}

	}

	for (size_t i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			meshData.get()->indices.emplace_back(face.mIndices[j]);
		}
	}

	// 본을 추가한다.
	if (mesh->HasBones())
	{
		for (size_t i = 0; i < mesh->mNumBones; ++i)
		{
			// 본을 채운다.
			std::unique_ptr<Bone> currentBone
				= std::make_unique<Bone>(mesh->mBones[i]->mName.C_Str(),
					GOE::FileManager::GetHash(mesh->mBones[i]->mName.C_Str()),
					currentMesh.get()->GetID());
			currentBone.get()->SetBoneIndex(i); // 본 인덱스 설정
			currentBone.get()->SetBoneOffset(aiMatrix4x4ToCoreMtrix(mesh->mBones[i]->mOffsetMatrix));//.Transpose()));
			currentBone.get()->SetNode(GOE::FileManager::GetHash(mesh->mBones[i]->mNode->mName.C_Str()));
			currentBone.get()->SetRootNode(GOE::FileManager::GetHash(mesh->mBones[i]->mArmature->mName.C_Str()));

			currentMesh.get()->AddBoneOffset(&currentBone.get()->GetBoneOffset());
			currentMesh.get()->AddBoneToMap(currentBone.get()->GetID(), currentBone.get());
			currentMesh.get()->AddBone(move(currentBone));

		}

		// 본을 순회한다.
		for (size_t i = 0; i < mesh->mNumBones; ++i)
		{
			meshData.get()->boneOffsets.push_back(aiMatrix4x4ToCoreMtrix(mesh->mBones[i]->mOffsetMatrix));// .Transpose()));
			// 현재 본이 영향을 주는 버텍스의 수만큼 순회한다.
			for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; ++j)
			{
				// 본이 영향을주는 버텍스아이디와 가중치를 가져온다.
				unsigned int vertexId = mesh->mBones[i]->mWeights[j].mVertexId;
				float weight = mesh->mBones[i]->mWeights[j].mWeight;

				/// 정점에 본데이터를 입력하는 과정이 필요함
				if (weight >= EPSILON)
				{
					// 빈 슬롯을 찾아서 본 인덱스와 가중치를 설정
					for (int k = 0; k < MAX_BONE; ++k)
					{
						// 가중치가 EPSILON보다작으면 아직 빈 슬롯이므로 여기에 본 정보 입력
						if (meshData.get()->vertices[vertexId].boneWeights[k] < EPSILON)
						{
							meshData.get()->vertices[vertexId].boneIndices[k] = i;
							meshData.get()->vertices[vertexId].boneWeights[k] = weight;
							break;
							// 빈 슬롯을 찾았으므로 루프 종료
							// 여기서 종료하지 않으면 동일 본이 여러 슬롯에 들어갈 수 있음
						}
					}
				}
			}
		}

		// 본가중치 정규화
		for (size_t i = 0; i < meshData->vertices.size(); ++i)
		{
			float totalWeight = 0.0f;
			// 가중치의 합을 구한다.
			for (int j = 0; j < MAX_BONE; ++j)
			{
				totalWeight += meshData->vertices[i].boneWeights[j];
			}
			// 가중치의 합이 EPSILON보다 크면 정규화 수행
			if (totalWeight > EPSILON)
			{
				// 각 가중치를 합으로 나누어 정규화
				for (int j = 0; j < MAX_BONE; ++j)
				{
					meshData->vertices[i].boneWeights[j] /= totalWeight;
				}
			}
		}
	}

	currentMesh.get()->SetMeshData(std::move(meshData));

	return currentMesh;
}

GOE::Matrix4x4 AssetLoader::aiMatrix4x4ToCoreMtrix(const aiMatrix4x4& nodeTM)
{
	GOE::Matrix4x4 l_tm;
	l_tm._11 = nodeTM.a1;
	l_tm._12 = nodeTM.a2;
	l_tm._13 = nodeTM.a3;
	l_tm._14 = nodeTM.a4;
	l_tm._21 = nodeTM.b1;
	l_tm._22 = nodeTM.b2;
	l_tm._23 = nodeTM.b3;
	l_tm._24 = nodeTM.b4;
	l_tm._31 = nodeTM.c1;
	l_tm._32 = nodeTM.c2;
	l_tm._33 = nodeTM.c3;
	l_tm._34 = nodeTM.c4;
	l_tm._41 = 0.f;//nodeTM.d1;
	l_tm._42 = 0.f;//nodeTM.d2;
	l_tm._43 = 0.f;//nodeTM.d3;
	l_tm._44 = 1.f;

	return l_tm.Transpose();
}
