#pragma once

/// <summary>
/// 리플렉션 시스템에서 지원하는 데이터 타입 식별자(Enum)와 관련 메타데이터를 정의합니다
/// </summary>
namespace GOE
{
	/// <summary>
	///  리플레션에서 지원할 자료들
	/// </summary>
	enum class EPropertyType
	{
		Bool,
		Int,
		Float,
		Double,
		String,

		// GOE Math Types
		Vector2,    // FloatVector2
		Vector3,    // FLoatVector3
		Vector4    // FLoatVector4
	};
}