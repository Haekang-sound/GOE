#pragma once
/// <summary>
/// 컴포넌트들의 최상위 인터페이스
/// 
/// IComponent를 상속받는 자료형은 반드시
/// 기본자료형과 GOE_Core에 있는 자료형으로 채우도록
/// 랜더러와의 분리 + 리플렉션데이터가 기본데이터와 GOE_Core를 기반으로하기 때문
/// </summary>
class IComponent
{
public:
	/// <summary>
	/// 자신과 오브젝트의 아이디는 필수
	/// </summary>
	/// <param name="ownerID">Entity ID</param>
	/// <param name="id">ComponentID</param>
	IComponent(size_t ownerID, size_t id)
		: m_id(id), m_ownerID(ownerID)	{}
	virtual ~IComponent() = default;

public:
	const size_t GetID() const { return m_id; }
	const size_t GetOwner() const { return m_ownerID; }
	inline const bool IsActive() const { return isActive; }

public: 
	inline void SetActive(bool active) { isActive = active; }

protected:
	const size_t m_id;
	const size_t m_ownerID; // entity ID

	bool isActive = true; // 활성화 여부
};
