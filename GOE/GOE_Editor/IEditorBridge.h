#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace Editor
{
	struct EntityInfo
	{
		size_t id = 0;
		std::string name = {};
		std::vector<EntityInfo> children;

	public:
		EntityInfo() = default;
		EntityInfo(size_t _id, std::string _name)
			: id(_id), name(_name){}
	};

	class IEditorBridge
	{
	public:
		virtual ~IEditorBridge() = default;

		// 씬의 모든 엔티티 목록 요청
		virtual std::vector<EntityInfo>& GetAllEntities() = 0;
		// 엔티티 선택 이벤트 전달
		virtual void OnEntitySelected(size_t entityID) = 0;
	};
}
