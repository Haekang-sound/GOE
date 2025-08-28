#pragma once
#include <DirectXMath.h>
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
			for(int i = 0 ; i < 4; ++i)
			{
				for(int j = 0; j < 4; ++j)
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
		XMFLOAT4 color = { 0,0,0,1 };
		XMFLOAT2 uv = { 0,0 }; // UV 좌표

	public:
		Vertex() = default;
		Vertex(XMFLOAT3 pos, XMFLOAT4 col)
		{
			position = pos;
			color = col;
			uv = { 0, 0 }; // UV 좌표 초기화
		}

		Vertex(const GOE::Vertex& other)
		{
			position.x = other.position[0];
			position.y = other.position[1];
			position.z = other.position[2];

			color.x = other.color[0];
			color.y = other.color[1];
			color.z = other.color[2];
			color.w = other.color[3];

			uv.x = other.uv[0];
			uv.y = other.uv[1];
		}

	public:
		Vertex& operator=(const GOE::Vertex& other)
		{

			position.x = other.position[0];
			position.y = other.position[1];
			position.z = other.position[2];

			color.x = other.color[0];
			color.y = other.color[1];
			color.z = other.color[2];
			color.w = other.color[3];

			uv.x = other.uv[0];
			uv.y = other.uv[1];
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
		}

	public:
		std::vector<Vertex> vertices; // 버텍스 정보
		std::vector<UINT32> indices; // 인덱스 정보

	public:
		MeshData& operator=(const GOE::MeshData& other)
		{
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
			return *this;
		}
	};
}