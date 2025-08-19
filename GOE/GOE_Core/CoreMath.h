#pragma once
#include <vector>
#include <cstdint>

namespace GOE
{
	struct Matrix4x4
	{
		union
		{
			float v[16];
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
				float _41, _42, _43, _44;
			};
			float m[4][4];
		};
	};

	struct FLoatVector3
	{
		float x;
		float y;
		float z;
	};

	struct FLoatVector4
	{
		float x;
		float y;
		float z;
		float w;
	};

	// 엔진에서 사용할 정점 데이터 구조체
	struct Vertex
	{
		// 위치, 법선, UV 등 필요한 데이터를 정의합니다.
		float position[3] = {};
		float color[4] = { 0,0,0,1 }; // 색상 (RGBA)
	};
	// 엔진에서 사용할 메시 데이터 구조체
	struct MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};



}