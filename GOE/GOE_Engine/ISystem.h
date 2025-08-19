#pragma once
class Scene;
namespace GOE
{
	struct EngineContext;
}

class ISystem
{
public:
	ISystem(Scene* scene, GOE::EngineContext* context)
		: m_scene(scene), m_context(context){}
	virtual ~ISystem() = default;

public:
	virtual void Initialize() {};
	virtual void Update() {};

public:
	inline const Scene* GetScene() const { return m_scene; }

protected:
	const Scene* m_scene;
	const GOE::EngineContext* m_context = nullptr; // 엔진 컨텍스트
};
