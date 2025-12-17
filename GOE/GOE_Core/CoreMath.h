#pragma once
#include <vector>
#include <cstdint>
#include <cmath> // 수학 함수 사용

// PI 값 정의
#define PI 3.1415926535f
#define MAX_BONE 4
#define EPSILON 1e-6f

// Degree를 Radian으로 변환
#define DEGREE_TO_RAD(degree) ((degree) * PI / 180.0f)

namespace GOE
{
	struct Matrix4x4;
	struct FloatVector2;
	struct FLoatVector3;
	struct FLoatVector4;

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

		// 왼손 좌표계 투영 행렬 생성 
		// 그래픽스 api에서 제공하는 api를 사용하자
		static Matrix4x4 PerspectiveFovLH(float fov, float aspectRatio, float nearZ, float farZ)
		{
			Matrix4x4 result = {}; // 0으로 초기화
			
			float sinFov = sin(0.5f * fov);
			float cosFov = cos(0.5f * fov);
			float height = cosFov / sinFov; // cot(fov/2)
			float width = height / aspectRatio;
			float fRange = farZ / (farZ - nearZ);

			result._11 = width;
			result._22 = height;
			result._33 = fRange;
			result._34 = 1.0f;
			result._43 = -fRange * nearZ;
			result._44 = 0.0f;

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

		// 그래픽스api에서 계산하는게
		// 효율적이기 때문에 쓰이진 않는다.
		Matrix4x4 Inverse()
		{
			float s0 = m[0][0] * m[1][1] - m[1][0] * m[0][1];
			float s1 = m[0][0] * m[1][2] - m[1][0] * m[0][2];
			float s2 = m[0][0] * m[1][3] - m[1][0] * m[0][3];
			float s3 = m[0][1] * m[1][2] - m[1][1] * m[0][2];
			float s4 = m[0][1] * m[1][3] - m[1][1] * m[0][3];
			float s5 = m[0][2] * m[1][3] - m[1][2] * m[0][3];

			float c5 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
			float c4 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
			float c3 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
			float c2 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
			float c1 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
			float c0 = m[2][0] * m[3][1] - m[3][0] * m[2][1];

			// 행렬식(Determinant) 계산
			float det = (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0);

			if (std::abs(det) <= EPSILON)
			{
				return Identity(); // 역행렬이 존재하지 않음 (단위 행렬 반환)
			}

			float invDet = 1.0f / det;
			Matrix4x4 result;

			result.m[0][0] = ( m[1][1] * c5 - m[1][2] * c4 + m[1][3] * c3) * invDet;
			result.m[0][1] = (-m[0][1] * c5 + m[0][2] * c4 - m[0][3] * c3) * invDet;
			result.m[0][2] = ( m[3][1] * s5 - m[3][2] * s4 + m[3][3] * s3) * invDet;
			result.m[0][3] = (-m[2][1] * s5 + m[2][2] * s4 - m[2][3] * s3) * invDet;

			result.m[1][0] = (-m[1][0] * c5 + m[1][2] * c2 - m[1][3] * c1) * invDet;
			result.m[1][1] = ( m[0][0] * c5 - m[0][2] * c2 + m[0][3] * c1) * invDet;
			result.m[1][2] = (-m[3][0] * s5 + m[3][2] * s2 - m[3][3] * s1) * invDet;
			result.m[1][3] = ( m[2][0] * s5 - m[2][2] * s2 + m[2][3] * s1) * invDet;

			result.m[2][0] = ( m[1][0] * c4 - m[1][1] * c2 + m[1][3] * c0) * invDet;
			result.m[2][1] = (-m[0][0] * c4 + m[0][1] * c2 - m[0][3] * c0) * invDet;
			result.m[2][2] = ( m[3][0] * s4 - m[3][1] * s2 + m[3][3] * s0) * invDet;
			result.m[2][3] = (-m[2][0] * s4 + m[2][1] * s2 - m[2][3] * s0) * invDet;

			result.m[3][0] = (-m[1][0] * c3 + m[1][1] * c1 - m[1][2] * c0) * invDet;
			result.m[3][1] = ( m[0][0] * c3 - m[0][1] * c1 + m[0][2] * c0) * invDet;
			result.m[3][2] = (-m[3][0] * s3 + m[3][1] * s1 - m[3][2] * s0) * invDet;
			result.m[3][3] = ( m[2][0] * s3 - m[2][1] * s1 + m[2][2] * s0) * invDet;

			*this = result;
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
		FloatVector2 operator* (const float& scalar)
		{
			FloatVector2 temp;
			temp.x = x * scalar;
			temp.y = y * scalar;
			return temp;
		}
		FloatVector2 operator- (const FloatVector2& other)
		{
			FloatVector2 temp;
			temp.x = x - other.x;
			temp.y = y - other.y;
			return temp;
		}
		FloatVector2& operator-= (const FloatVector2& other)
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


	struct FLoatVector4
	{
		float x;
		float y;
		float z;
		float w;
	public:
		FLoatVector4() = default;
		FLoatVector4(float x, float y, float z, float w)
			: x(x), y(y), z(z), w(w)
		{
		}

	public:
		Matrix4x4 ToRotationMatrix() const
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

		// 헬퍼 함수: 내적 (Dot Product)
		float Dot(const FLoatVector4& other) const
		{
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}

		// 헬퍼 함수: 정규화 (Normalize)
		void Normalize()
		{
			float lenSq = x * x + y * y + z * z + w * w;
			if (lenSq > EPSILON)
			{
				float len = sqrt(lenSq);
				x /= len;
				y /= len;
				z /= len;
				w /= len;
			}
		}

		// Slerp 함수 직접 구현
		static FLoatVector4 Slerp(FLoatVector4 a, FLoatVector4 b, float t)
		{
			// 1. 두 쿼터니언 사이의 "각도"의 코사인 값을 계산합니다 (내적).
			float cosTheta = a.Dot(b);

			// 2."최단 경로" 보정
			// 만약 내적이 음수면, 두 쿼터니언은 180도가 넘는 경로에 있습니다.
			// 한쪽 쿼터니언을 뒤집으면(-b) 180도 이내의 "최단 경로"로 보간할 수 있습니다.
			// (쿼터니언 q와 -q는 동일한 회전을 나타냅니다)
			if (cosTheta < 0.0f)
			{
				// b를 뒤집고, 코사인 값도 부호를 바꿉니다.
				b.x = -b.x; b.y = -b.y; b.z = -b.z; b.w = -b.w;
				cosTheta = -cosTheta;
			}

			// 3. (중요) "거의 같은" 쿼터니언 보정 (0으로 나누기 방지)
			// 만약 cosTheta가 1.0에 매우 가까우면 (두 쿼터니언이 거의 같음),
			// sin(theta)가 0이 되어 0으로 나누기 오류가 발생합니다.
			// 이 경우, Slerp 대신 Lerp(선형 보간)를 사용해도 시각적으로 문제가 없습니다.
			if (cosTheta > 1.f - EPSILON)
			{
				// Lerp: (1-t)*a + t*b
				FLoatVector4 result = a * (1.0f - t) + b * t;
				result.Normalize(); // Lerp 결과는 길이가 1이 아니므로 정규화
				return result;
			}

			// 4. 실제 Slerp 계산
			// 각도(theta)를 구합니다.
			float theta = acos(cosTheta);
			// sin(theta) 값을 구합니다 (0이 아님이 보장됨).
			float sinTheta = sin(theta);

			// 5. 두 쿼터니언에 적용할 가중치(스케일)를 계산합니다.
			float scaleA = sin((1.0f - t) * theta) / sinTheta;
			float scaleB = sin(t * theta) / sinTheta;

			// 6. 가중치를 적용하여 최종 쿼터니언을 계산합니다.
			FLoatVector4 result;
			result.x = scaleA * a.x + scaleB * b.x;
			result.y = scaleA * a.y + scaleB * b.y;
			result.z = scaleA * a.z + scaleB * b.z;
			result.w = scaleA * a.w + scaleB * b.w;

			return result;
		}

	public:
		FLoatVector4& operator*= (const float& scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			w *= scalar;
			return *this;
		}
		FLoatVector4 operator * (const float& scalar)
		{
			FLoatVector4 temp;
			temp.x = x * scalar;
			temp.y = y * scalar;
			temp.z = z * scalar;
			temp.w = w * scalar;
			return temp;
		}
		FLoatVector4& operator+= (const FLoatVector4& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			w += other.w;
			return *this;
		}
		FLoatVector4 operator + (const FLoatVector4& other)
		{
			FLoatVector4 temp;
			temp.x = x + other.x;
			temp.y = y + other.y;
			temp.z = z + other.z;
			temp.w = w + other.w;
			return temp;
		}
		FLoatVector4& operator-= (const FLoatVector4& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			w -= other.w;
			return *this;
		}
		FLoatVector4 operator- (const FLoatVector4& other)
		{
			FLoatVector4 temp;
			temp.x = x - other.x;
			temp.y = y - other.y;
			temp.z = z - other.z;
			temp.w = w - other.w;
			return temp;
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
		Matrix4x4 ToRotationMatrixEuler() const
		{
			return Matrix4x4::RotationX(x) * Matrix4x4::RotationY(y) * Matrix4x4::RotationZ(z);
		}

		Matrix4x4 ToTranslationMatrix() const
		{
			return Matrix4x4::Translation(x, y, z);
		}
		

	public:
		FLoatVector3 operator= (const FLoatVector3 other)
		{
			FLoatVector3 temp;
			x = other.x;
			y = other.y;
			z = other.z;
			return temp;
		}

		FLoatVector3& operator*= (const float& scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			return *this;
		}
		FLoatVector3 operator* (const float& scalar)
		{
			FLoatVector3 temp;
			temp.x = x * scalar;
			temp.y = y * scalar;
			temp.z = z * scalar;
			return temp;
		}

		FLoatVector3& operator-= (const FLoatVector3& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		FLoatVector3 operator- (const FLoatVector3& other)
		{
			FLoatVector3 temp;
			temp.x = x - other.x;
			temp.y = y - other.y;
			temp.z = z - other.z;
			return temp;
		}

		FLoatVector3 operator+ (const FLoatVector3& other)
		{
			FLoatVector3 temp;
			temp.x = x + other.x;
			temp.y = y + other.y;
			temp.z = z + other.z;
			return temp;
		}

		FLoatVector3& operator+= (const FLoatVector3& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		FLoatVector3& operator+= (const FLoatVector4& other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		FLoatVector3& operator-= (const FLoatVector4& other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
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
		unsigned int boneIndices[MAX_BONE] = {};
		float boneWeights[MAX_BONE] = {};

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
