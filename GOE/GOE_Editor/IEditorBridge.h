#pragma once
#include <string>
#include <vector>

namespace Editor
{
	struct EntityInfo
	{
		size_t id;
		std::string name;
	};

	class IEditorBridge
	{
	public:
		virtual ~IEditorBridge() = default;

		// 씬의 모든 엔티티 목록 요청
		virtual std::vector<EntityInfo> GetAllEntities() = 0;
		// 엔티티 선택 이벤트 전달
		virtual void OnEntitySelected(size_t entityID) = 0;
	};
}
