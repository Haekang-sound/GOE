#pragma once
#include <vector>
#include <cstdint>

// PI 값 정의
#define PI 3.1415926535f
#define MAX_BONE 4

// Degree를 Radian으로 변환하는 매크로
#define DEGREE_TO_RAD(degree) ((degree) * PI / 180.0f)

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

	public:
		// 단위 행렬을 반환합니다.
		static Matrix4x4 Identity()
		{
			Matrix4x4 identity = {};
			identity._11 = 1;
			identity._22 = 1;
			identity._33 = 1;
			identity._44 = 1;
			return identity;
		}

		// 이동 행렬을 생성합니다.
		static Matrix4x4 Translation(float x, float y, float z)
		{
			Matrix4x4 result = Identity();
			result._41 = x;
			result._42 = y;
			result._43 = z;
			return result;
		}

		// X축 기준 회전 행렬을 생성합니다. (라디안 값 사용)
		static Matrix4x4 RotationX(float radians)
		{
			Matrix4x4 result = Identity();
			result._22 = cos(radians);
			result._23 = sin(radians);
			result._32 = -sin(radians);
			result._33 = cos(radians);
			return result;
		}

		// Y축 기준 회전 행렬을 생성합니다. (라디안 값 사용)
		static Matrix4x4 RotationY(float radians)
		{
			Matrix4x4 result = Identity();
			result._11 = cos(radians);
			result._13 = -sin(radians);
			result._31 = sin(radians);
			result._33 = cos(radians);
			return result;
		}

		// Z축 기준 회전 행렬을 생성합니다. (라디안 값 사용)
		static Matrix4x4 RotationZ(float radians)
		{
			Matrix4x4 result = Identity();
			result._11 = cos(radians);
			result._12 = sin(radians);
			result._21 = -sin(radians);
			result._22 = cos(radians);
			return result;
		}

		// 크기 조절 행렬을 생성합니다.
		static Matrix4x4 Scaling(float x, float y, float z)
		{
			Matrix4x4 result = Identity();
			result._11 = x;
			result._22 = y;
			result._33 = z;
			return result;
		}
	public:
		Matrix4x4 Transpose()
		{
			Matrix4x4 temp = *this;
			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					m[i][j] = temp.m[j][i];
				}
			}
			return *this;
		}

	public:
		// 행렬 곱셈 연산자
		Matrix4x4 operator* (const Matrix4x4& other) const
		{
			Matrix4x4 result;
			for (int i = 0; i < 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					result.m[i][j] = m[i][0] * other.m[0][j] +
						m[i][1] * other.m[1][j] +
						m[i][2] * other.m[2][j] +
						m[i][3] * other.m[3][j];
				}
			}
			return result;
		}
		Matrix4x4& operator*= (const Matrix4x4& other)
		{
			*this = *this * other;
			return *this;
		}

	};

	struct FloatVector2
	{
		float x=0.0;
		float y=0.0;
	public:
		FloatVector2() = default;
		FloatVector2(float x, float y)
			: x(x), y(y){}

	public:
		FloatVector2& operator* (const float& other)
		{
			x *= other;
			y *= other;
			return *this;
		}
		FloatVector2& operator- (const FloatVector2& other)
		{
			x -= other.x;
			y -= other.y;
			return *this;
		}
		FloatVector2& operator+= (const FloatVector2& other)
		{
			x += other.x;
			y += other.y;
			return *this;
		}

	};

	struct FLoatVector3
	{
		float x;
		float y;
		float z;
	public:
		FLoatVector3() = default;
		FLoatVector3(float x, float y, float z)
			: x(x), y(y), z(z){}

	public:
		Matrix4x4 ToScaleMatrix() const
		{
			return Matrix4x4::Scaling(x, y, z);
		}

		// 오일러 회전 일경우
		Matrix4x4 ToRotationXMatrix() const
		{
			return Matrix4x4::RotationX(x) * Matrix4x4::RotationY(y) * Matrix4x4::RotationZ(z);
		}

		Matrix4x4 ToTranslationMatrix() const
		{
			return Matrix4x4::Translation(x, y, z);
		}
		

	public:
		FLoatVector3& operator* (const float& other)
		{
			x *= other;
			y *= other;
			z *= other;
			return *this;
		}
		FLoatVector3& operator- (const FLoatVector3& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}
		FLoatVector3& operator+= (const FLoatVector3& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

	};

	struct FLoatVector4
	{
		float x;
		float y;
		float z;
		float w;
	public:
		FLoatVector4() = default;
		FLoatVector4(float x, float y, float z, float w)
			: x(x), y(y), z(z), w(w){}

	public: 
		Matrix4x4 ToRotationXMatrix() const
		{
			// 쿼터니언을 회전 행렬로 변환
			Matrix4x4 result = Matrix4x4::Identity();

			result._11 = 1 - 2 * (y * y + z * z);
			result._12 = 2 * (x * y + z * w);
			result._13 = 2 * (x * z - y * w);

			result._21 = 2 * (x * y - z * w);
			result._22 = 1 - 2 * (x * x + z * z);
			result._23 = 2 * (y * z + x * w);

			result._31 = 2 * (x * z + y * w);
			result._32 = 2 * (y * z - x * w);
			result._33 = 1 - 2 * (x * x + y * y);

			return result;
		}

	public:
		FLoatVector4& operator* (const float& scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			w *= scalar;
			return *this;
		}
		FLoatVector4& operator+= (const FLoatVector4& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			w += other.w;
			return *this;
		}
		FLoatVector4& operator-= (const FLoatVector4& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			w -= other.w;
			return *this;
		}
	};

	// 엔진에서 사용할 정점 데이터 구조체
	struct Vertex
	{
		// 위치, 법선, UV 등 필요한 데이터를 정의합니다.
		FLoatVector3 position = {};
		FLoatVector4 color = { 1,1,1,1 }; // 색상 (RGBA)
		FloatVector2 uv = { 0,0 }; // UV 좌표
		FLoatVector3 normal = { 0,0,0 }; // 법선 벡터
		unsigned int boneIndices[MAX_BONE];
		float boneWeights[MAX_BONE];

		// 16바이트 정렬을 위한 패딩
		FloatVector2 padding = {}; 
	};

	// 엔진에서 사용할 메시 데이터 구조체
	struct MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<Matrix4x4> boneOffsets;
	};


}
