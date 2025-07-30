#pragma once
#include <vector>

class Mesh;
/// <summary>
/// 모델링에 필요한 
/// 정보를 갖고 있는 클래스
/// 
/// ohk 2025.07.29
/// </summary>
class Model
{
public:
	Model() = default;
	~Model() = default;
	// 모델링에 필요한 정보를 갖고 있는 클래스
	// 예: 버텍스, 인덱스, 텍스처 등

private:
	std::string m_name; // 모델 이름
	std::vector<Mesh*> m_meshes; // 버텍스 정보


};

