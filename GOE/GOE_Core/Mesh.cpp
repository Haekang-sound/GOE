#include "Core_pch.h"
#include "Commons.h"
#include "Mesh.h"

// 소멸자 정의 (컴파일러는 이 시점에서 MeshData의 전체 정의를 알 수 있음)
Mesh::~Mesh() = default;
//
//const GOE::MeshData& Mesh::GetMeshData()
//{
//	if (m_meshData)
//	{
//		return *m_meshData.get();
//	}
//	else
//	{
//		throw std::runtime_error("MeshData is not initialized.");
//	}
//}