#pragma once
class Node
{
public:
	Node() = default;
	Node(const std::string& name, size_t id)
		: m_name(name), m_id(id){}

	~Node() = default;

public:	
	inline const std::string& GetName() const { return m_name; }
	inline size_t GetID() const { return m_id; }

	// 이동생성으로 노드를 받아오기
	inline void AddChild(std::unique_ptr<Node>&& child) { m_children.push_back(std::move(child));}
	inline const std::vector<std::unique_ptr<Node>>& GetChildren() const { return m_children; }

	inline void AddMeshIndex(size_t meshIndex) { m_meshIndex.push_back(meshIndex); }
	inline const std::vector<size_t>& GetMeshIndex() const { return m_meshIndex; }

private:
	const std::string m_name; // 노드 이름
	const size_t m_id; // 노드 ID

	std::vector<std::unique_ptr<Node>> m_children;
	std::vector<size_t> m_meshIndex; // 이 노드가 참조하는 메쉬의 인덱스

};

