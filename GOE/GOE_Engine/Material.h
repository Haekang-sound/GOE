#pragma once
#include "IComponent.h"
class Material : public IComponent
{
public:
	Material(size_t ownerID, size_t id);
	~Material();

public:
	const size_t GetTextureID() const { return m_textureID; }
	const std::string& GetName() const { return m_name; }

public:
	void SetTextureID(size_t textureID) { m_textureID = textureID; }
	void SetName(const std::string name) { m_name = name; }

protected:
	/// 이걸가져가서 
	size_t m_textureID = 0;

	std::string m_name = ""; // 오브젝트 이름
};

