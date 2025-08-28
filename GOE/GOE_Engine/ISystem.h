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
	virtual void Update(double dTime) {};
	virtual void DebugUpdate(double dTime) {};


public:
	inline Scene* GetScene() { return m_scene; }
	inline const Scene* GetScene() const { return m_scene; }

protected:
	Scene* m_scene;
	const GOE::EngineContext* m_context = nullptr; // 엔진 컨텍스트
};
