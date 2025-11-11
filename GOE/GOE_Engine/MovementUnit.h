#pragma once
#include "IComponent.h"

class Entitiy;

class MovementUnit : public IComponent
{
public:
	MovementUnit(size_t ownerID, size_t id);
	~MovementUnit();

public:
	inline const bool IsMoveable() const { return m_isMoveable; }

public:
	inline  void SetMoveable(bool isMoveable) { m_isMoveable = isMoveable; }

private:
	bool m_isMoveable = true;
};

