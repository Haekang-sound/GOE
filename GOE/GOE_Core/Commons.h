#pragma once
#include <vector>
#include <cstdint>

// 엔진에서 사용할 정점 데이터 구조체
struct VertexK
{
	// 위치, 법선, UV 등 필요한 데이터를 정의합니다.
	float position[3];
	float color[4] = { 1,0,1,1 }; // 색상 (RGBA)
};

// 엔진에서 사용할 메시 데이터 구조체
struct MeshData
{
	std::vector<VertexK> vertices;
	std::vector<uint32_t> indices;
};