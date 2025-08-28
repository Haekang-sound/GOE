#include "Engine_pch.h"
#include "Engine.h"
#include "../GOE_Core/Window.h"
#include "../GOE_Editor/EditorCore.h"
#include "../GOE_Editor/DebugManager.h"
#include "SceneManager.h"

GOE::Engine::Engine(HINSTANCE hInst, int nCmdShow)
	: m_hInst(hInst), m_nCmdShow(nCmdShow),
	m_winCore(nullptr), m_renderer(nullptr),
	m_editor(nullptr), m_assetCore(nullptr),
	m_sceneManager(nullptr)
{
	m_sceneManager = std::make_unique<SceneManager>();
}

GOE::Engine::~Engine() = default;

void GOE::Engine::Initialize()
{
	// 윈도우 초기화
	m_winCore = std::make_unique<Window>(L"GOE", 1200, 800, m_hInst, m_nCmdShow);
	m_winCore->InitInstance();
	m_winCore->SetExternalMsgHandler(&ImGui_ImplWin32_WndProcHandler);

	// 에셋코어 초기화, 모델로드
	// fbx파일을 로드합니다.
	m_assetCore = std::make_unique<AssetCore>();
	m_assetCore.get()->CreateAssetLoader();
	m_assetCore.get()->LoadModel("D:\\project\\GOE\\GOE\\Assets\\models\\Ch03_nonPBR.fbx");
	m_assetCore.get()->LoadModel("D:\\project\\GOE\\GOE\\Assets\\models\\kuramon.fbx");
	m_assetCore.get()->LoadTextureFromFile("D:\\project\\GOE\\GOE\\Assets\\models\\kuramon.fbx");

	// 렌더러 초기화
	m_renderer = std::make_unique<GOERenderer>(m_winCore->GetHWND());
	m_renderer->OnInit();
	m_renderer.get()->LoadTexture("D:\\project\\GOE\\GOE\\Assets\\textures\\chr629a01.png");

	/// 로드된 매쉬중 원하는 메쉬를 이름으로 골라서
	/// 그래픽스 리소스를 생성합니다.
	std::hash<std::string> hasher;
	m_renderer->CreateOneMeshResource(m_assetCore.get()->GetMesh(hasher("Ch03"))); // 첫번째 메쉬를 가져와서 리소스 생성
	m_renderer->CreateOneMeshResource(m_assetCore.get()->GetMesh(hasher("chr629_0"))); // 첫번째 메쉬를 가져와서 리소스 생성
	m_renderer->CreateOneMeshResource(m_assetCore.get()->GetMesh(hasher("chr629_1"))); // 첫번째 메쉬를 가져와서 리소스 생성
	m_renderer->CopyUploadHeapToDefault();

	//// 에디터 초기화
	m_editor = std::make_unique<EditorCore>(m_winCore->GetHWND());
	m_editor->Initialize(m_renderer.get()->GetUIInfo());	

	m_context = std::make_unique<GOE::EngineContext>();
	
	// 컨텍스트 등록
	m_context.get()->renderer = m_renderer.get();
	m_context.get()->assetCore = m_assetCore.get();

	/// 엔진관련 초기화
	m_sceneManager.get()->Initialize(m_context.get());

}

void GOE::Engine::OnUpdate(double dTime) 
{
	InputUpdate();

	m_sceneManager.get()->OnUpdate(dTime);

	m_editor->OnUpdate();
	m_renderer->OnUpdate();

	DebugUpdate();	
}

void GOE::Engine::BeginRender()
{
	m_renderer->BeginRender();
}

void GOE::Engine::OnRender()
{
	m_renderer->OnRender(); // 렌더링 호출
	m_editor->OnRender(m_renderer.get()->GetUILoopInfo());
}

void GOE::Engine::EndRender()
{
	m_renderer->EndRender();
}

/// <summary>
/// rule of 0를 추구하기때문에
/// 아직은 괜찮아..
/// </summary>
void GOE::Engine::Release(){}


void GOE::Engine::DebugUpdate()
{
	m_sceneManager.get()->DebugUpdate();
	DebugManager::GetInstance().OnDebugUpdate();
}

void GOE::Engine::InputUpdate()
{
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) 
		DestroyWindow(m_winCore->GetHWND());
}