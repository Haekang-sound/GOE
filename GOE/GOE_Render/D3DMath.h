#pragma once
#include <DirectXMath.h>
#include "CoreMath.h"

using namespace DirectX;

namespace Graphics
{
	struct Matrix4x4
	{
	public:
		XMFLOAT4X4 matrix; // 64바이트(행렬)
	public:
		Matrix4x4() = default;
		Matrix4x4(GOE::Matrix4x4 core_matrix)
		{
			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					matrix.m[i][j] = core_matrix.m[i][j];
				}
			}
		}

	public:
		Matrix4x4& operator=(const GOE::Matrix4x4& other)
		{
			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					matrix.m[i][j] = other.m[i][j];
				}
			}
			return *this;
		}
	};


	/// <summary>z
	/// 버텍스 구조체
	/// </summary>
	struct Vertex
	{
		XMFLOAT3 position = {};
		XMFLOAT4 color = { 1,1,1,1 };
		XMFLOAT2 uv = { 0,0 }; // UV 좌표
		XMFLOAT3 normal = { 0,0,0 }; // 법선 벡터
		XMUINT4 boneIndices = {};
		XMFLOAT4 boneWeights = {};
		// 16바이트 정렬을 위한 패딩
		XMFLOAT2 padding = {};

	public:
		Vertex() = default;
		Vertex(XMFLOAT3 pos, XMFLOAT4 col)
		{
			position = pos;
			color = col;
			uv = { 0, 0 }; // UV 좌표 초기화
			normal = { 0, 0, 0 }; // 법선 벡터 초기화
		}

		Vertex(const GOE::Vertex& other)
		{
			position.x = other.position.x;
			position.y = other.position.y;
			position.z = other.position.z;

			color.x = other.color.x;
			color.y = other.color.y;
			color.z = other.color.z;
			color.w = other.color.w;

			uv.x = other.uv.x;
			uv.y = other.uv.y;

			normal.x = other.normal.x;
			normal.y = other.normal.y;
			normal.z = other.normal.z;

			boneIndices.w = other.boneIndices[0];
			boneIndices.x = other.boneIndices[1];
			boneIndices.y = other.boneIndices[2];
			boneIndices.z = other.boneIndices[3];

			boneWeights.w = other.boneWeights[0];
			boneWeights.x = other.boneWeights[1];
			boneWeights.y = other.boneWeights[2];
			boneWeights.z = other.boneWeights[3];
		}

	public:
		Vertex& operator=(const GOE::Vertex& other)
		{

			position.x = other.position.x;
			position.y = other.position.y;
			position.z = other.position.z;

			color.x = other.color.x;
			color.y = other.color.y;
			color.z = other.color.z;
			color.w = other.color.w;

			uv.x = other.uv.x;
			uv.y = other.uv.y;

			normal.x = other.normal.x;
			normal.y = other.normal.y;
			normal.z = other.normal.z;

			boneIndices.w = other.boneIndices[0];
			boneIndices.x = other.boneIndices[1];
			boneIndices.y = other.boneIndices[2];
			boneIndices.z = other.boneIndices[3];

			boneWeights.w = other.boneWeights[0];
			boneWeights.x = other.boneWeights[1];
			boneWeights.y = other.boneWeights[2];
			boneWeights.z = other.boneWeights[3];

			return *this;
		}

	};


	/// <summary>
	/// 버텍스와 인덱스 정보를 갖고 있는 
	/// 메쉬 구조체
	/// 
	/// </summary>
	struct MeshData
	{
	public:
		MeshData() = default;
		MeshData(const GOE::MeshData& other)
		{
			*this = other;
			vertices.clear();
			indices.clear();
			for (const auto& vertex : other.vertices)
			{
				vertices.emplace_back(vertex);
			}
			for (const auto& index : other.indices)
			{
				indices.push_back(index);
			}
			for (const auto& matrix : other.boneOffsets)
			{
				XMFLOAT4X4 temp;
				temp._11 = matrix._11;
				temp._12 = matrix._12;
				temp._13 = matrix._13;
				temp._14 = matrix._14;

				temp._21 = matrix._21;
				temp._22 = matrix._22;
				temp._23 = matrix._23;
				temp._24 = matrix._24;

				temp._31 = matrix._31;
				temp._32 = matrix._32;
				temp._33 = matrix._33;
				temp._34 = matrix._34;

				temp._41 = matrix._41;
				temp._42 = matrix._42;
				temp._43 = matrix._43;
				temp._44 = matrix._44;

				boneOffsets.push_back(temp);
			}
		}

	public:
		std::vector<Vertex> vertices; // 버텍스 정보
		std::vector<UINT32> indices; // 인덱스 정보
		std::vector<XMFLOAT4X4> boneOffsets;

	public:
		MeshData& operator=(const GOE::MeshData& other)
		{
			vertices.clear();
			indices.clear();
			boneOffsets.clear();
			for (const auto& vertex : other.vertices)
			{
				vertices.emplace_back(vertex);
			}
			for (const auto& index : other.indices)
			{
				indices.push_back(index);
			}
			/*for (const auto& matrix : other.boneOffsets)
			{
				XMFLOAT4X4 temp;
				temp._11 = matrix._11;
				temp._12 = matrix._12;
				temp._13 = matrix._13;
				temp._14 = matrix._14;

				temp._11 = matrix._21;
				temp._12 = matrix._22;
				temp._13 = matrix._23;
				temp._14 = matrix._24;

				temp._11 = matrix._31;
				temp._12 = matrix._32;
				temp._13 = matrix._33;
				temp._14 = matrix._34;

				temp._11 = matrix._41;
				temp._12 = matrix._42;
				temp._13 = matrix._43;
				temp._14 = matrix._44;

				boneOffsets.push_back(temp);

			}*/
			return *this;
		}
	};

	struct CB
	{
		XMFLOAT4X4 world = {};
		XMFLOAT4X4 viewProjection = {};
		XMFLOAT3   cameraPosition = {};
		float      padding = 0.f; // 16바이트 정렬을 위한 패딩
	};

	struct SkinningCB
	{
		XMFLOAT4X4 boneTransforms[128]; // 최대 128개의 본 변환 행렬
	};

	struct BoneOffsetCB
	{
		XMFLOAT4X4 offsetTransforms[128]; // 최대 128개의 본 오프셋 행렬
	};
}