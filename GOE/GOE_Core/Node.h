#pragma once
class Node
{
public:
	//Node() = default;
	Node(const std::string& name, size_t id)
		: m_parent(nullptr), m_name(name), m_id(id)	{}
	~Node();

public:
	inline const std::string& GetName() const { return m_name; }
	inline size_t GetID() const { return m_id; }
	// 이동생성으로 자식노드를 추가한다.
public:
	inline Node* GetParent() const { return m_parent; }
	inline const std::vector<std::unique_ptr<Node>>& GetChildren() const { return m_children; }
	inline const std::vector<size_t>& GetMeshIndex() const { return m_meshIndex; }
	inline GOE::Matrix4x4& GetLocalTM() { return m_localTM; }
	/// 반드시 setWorldTM함수를 호출한 이후에 사용해야한다.
	inline GOE::Matrix4x4 GetWorldTM() { return m_worldTM; }
	inline GOE::Matrix4x4 GetNodePositionMatrix()
	{
		return GOE::Matrix4x4::Translation(
			m_nodePosition.x,
			m_nodePosition.y,
			m_nodePosition.z);
	}

public:
	inline void SetParent(Node* parent) { m_parent = parent; }
	inline void SetModelID(size_t id) { m_modelID = id; }
	inline void AddChild(std::unique_ptr<Node>&& child) { m_children.push_back(std::move(child)); }
	inline void AddMeshIndex(size_t meshIndex) { m_meshIndex.push_back(meshIndex); }
	inline void SetNodeIndex(int i) { m_index = i; }
	inline void SetLocalTM(GOE::Matrix4x4 transform){ m_localTM = transform; }	
	inline void SetWorldTM(GOE::Matrix4x4 transform) { m_worldTM = transform; }
	inline void SetNodePosition(float x, float y, float z) 
	{
		m_nodePosition.x = x; 
		m_nodePosition.y = y; 
		m_nodePosition.z = z; 
	}

private:
	const std::string m_name; // 노드 이름
	const size_t m_id; // 노드 ID
	size_t m_modelID = 0;
	int m_index = -1;

	/// <summary>
	/// 스키닝 하는 중
	/// 노드는 각각 로컬한 트랜스폼을 갖고 있다.
	/// 노드의 위치는 부모의 트랜스폼도 받아와dld 서 그 위에 존재해야함
	/// 그리고 이 노드에 애니메이션 값이 곱해질 것이고
	/// 
	/// </summary>
	GOE::Matrix4x4 m_localTM = GOE::Matrix4x4::Identity();
	GOE::Matrix4x4 m_worldTM = GOE::Matrix4x4::Identity();
	GOE::FLoatVector3 m_nodePosition = GOE::FLoatVector3(0.0f, 0.0f, 0.0f);

	Node* m_parent = nullptr;
	std::vector<std::unique_ptr<Node>> m_children;
	std::vector<size_t> m_meshIndex; // 이 노드가 참조하는 메쉬의 인덱스

};

