#pragma once
#include <DirectXMath.h>
using namespace DirectX;

namespace Graphics
{
	struct MVP
	{
		XMFLOAT4X4 mvp; // 64바이트(행렬)
	};

	/// <summary>
	/// 버텍스 구조체
	/// </summary>
	struct Vertex
	{
		XMFLOAT3 position;
		XMFLOAT4 color = { 0,0,0,1 };

	public:
		Vertex() = default;

		Vertex(XMFLOAT3 pos, XMFLOAT4 col)
		{
			position = pos;
			color = col;
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
			return *this;
		}

	};
}