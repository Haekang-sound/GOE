#pragma once
#include "IComponent.h"

class Entitiy;

/// <summary>
/// IComponent를 상속받은
/// Transform 컴포넌트 트렌스폼 정보를 갖고 있다.
/// 
/// </summary>
class Transform : public IComponent
{
public:
	Transform(size_t ownerID, size_t id);
	~Transform();

public:
	inline const GOE::Matrix4x4& GetLocalTM() const { return m_localTM; }
	inline GOE::Matrix4x4& GetLocalTM() { return m_localTM; }
	inline const GOE::Matrix4x4& GetParentTM() const { return m_parentTM; }

	inline const GOE::FLoatVector3& GetScale() const { return m_scale; }
	inline const GOE::FLoatVector3& GetRotation() const { return m_rotation; }
	inline const GOE::FLoatVector3& GetPosition() const { return m_position; }

	GOE::FLoatVector4 GetRightVector();	
	GOE::FLoatVector4 GetUpVector();
	GOE::FLoatVector4 GetForwardVector();

public:
	inline void SetLocalTM(GOE::Matrix4x4 TM) { m_localTM = TM; }
	inline void SetParentTM(GOE::Matrix4x4 TM) { m_parentTM = TM; }

	inline void SetScale(GOE::FLoatVector3 v) 
	{
		m_scale = v;
		UpdateTransform();
	}
	// 기존 함수명 유지 (호환성)
	inline void SetScaleTM(GOE::FLoatVector3 v) { SetScale(v); }

	inline void SetRotation(GOE::FLoatVector3 v)
	{
		m_rotation = v; 
		UpdateTransform();
	}
	inline void SetRotationTM(GOE::FLoatVector3 v) { SetRotation(v); }

	inline void SetPosition(GOE::FLoatVector3 v)
	{ 
		m_position = v; 
		UpdateTransform();
	}
	inline void SetPositionTM(GOE::FLoatVector3 v) { SetPosition(v); }

private:
	void UpdateTransform()
	{
		GOE::Matrix4x4 S = GOE::Matrix4x4::Scaling(m_scale.x, m_scale.y, m_scale.z);
		GOE::Matrix4x4 R = m_rotation.ToRotationMatrixEuler();
		GOE::Matrix4x4 T = GOE::Matrix4x4::Translation(m_position.x, m_position.y, m_position.z);
		m_localTM = S * R * T;
	}

protected:
	GOE::Matrix4x4 m_localTM;
	GOE::Matrix4x4 m_parentTM;

	GOE::FLoatVector3 m_scale;
	GOE::FLoatVector3 m_rotation;
	GOE::FLoatVector3 m_position;

};

