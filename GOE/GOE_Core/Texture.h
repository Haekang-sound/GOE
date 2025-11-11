#pragma once

class Texture
{
public:
	Texture() = default;
	Texture(size_t hash) : m_id(hash) {}

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	Texture(Texture&&) noexcept = default;
	Texture& operator=(Texture&&) noexcept = default;

	~Texture();

private:
	std::string m_name = ""; // 모델 이름
	const size_t m_id;		// 모델 ID

	std::unique_ptr<Node> m_rootNode;
	std::vector<MESH_ID> m_meshIDs;
	std::unordered_map<MESH_ID, MESH_INDEX> m_meshMap; // 매쉬 이름과 인덱스를 매핑하는 해시맵
};

