#pragma once
#include <vector>

struct MeshData;

/// <summary>
/// 버텍스와 인덱스정보를 갖고 있는 매쉬
/// 
/// ohk 2025.07.29
/// </summary>
class Mesh
{
	public:
	Mesh() = default;
	~Mesh() = default;

private:
	std::string m_name; // 매쉬 이름
	MeshData* m_meshData = nullptr; // 매쉬 데이터
};

