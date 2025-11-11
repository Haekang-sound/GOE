#include "Engine_pch.h"
#include "Engine.h"
#include "DebugManager.h"
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

	///애니메이션 로드
	m_assetCore.get()->LoadAnimation("D:\\project\\GOE\\GOE\\Assets\\animation\\Samba Dancing.fbx");
	m_assetCore.get()->LoadAnimation("D:\\project\\GOE\\GOE\\Assets\\models\\kuramon.fbx");

	// 렌더러 초기화
	m_renderer = std::make_unique<GOERenderer>(m_winCore->GetHWND());
	m_renderer->OnInit();
	m_renderer.get()->LoadTexture("D:\\project\\GOE\\GOE\\Assets\\textures\\Ch03_1001_Diffuse.png");
	m_renderer.get()->LoadTexture("D:\\project\\GOE\\GOE\\Assets\\textures\\chr629a01.png");

	/// 로드된 매쉬중 원하는 메쉬를 이름으로 골라서
	/// 그래픽스 리소스를 생성합니다.
	// 메쉬 리소스생성 
	m_renderer->CreateOneMeshResource(m_assetCore.get()->GetMesh(GOE::FileManager::GetHash("Ch03")));
	m_renderer->CreateOneMeshResource(m_assetCore.get()->GetMesh(GOE::FileManager::GetHash("chr629_0")));
	m_renderer->CreateOneMeshResource(m_assetCore.get()->GetMesh(GOE::FileManager::GetHash("chr629_1")));
	// 애니매이션 리소스 생성
	m_renderer->CopyUploadHeapToDefault();

	// 에디터 초기화
	m_editor = std::make_unique<EditorCore>(m_winCore->GetHWND());
	m_editor->Initialize(m_renderer.get()->GetUIInfo());

	m_context = std::make_unique<GOE::EngineContext>();

	// 컨텍스트 등록
	// 엔진의 여러 서브시스템들이 접근할 수 있도록
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

#if defined(_DEBUG)
	DebugUpdate();
#endif
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
void GOE::Engine::Release() {}


void GOE::Engine::DebugUpdate()
{
	m_sceneManager.get()->DebugUpdate();
	DebugManager::GetInstance().OnDebugUpdate();
}

/// 창부수기! 
/// 현재는 프로그램 종료용도로만 사용중
/// 이제 TIME과 INPUT정도는 만들어야함
/// 
void GOE::Engine::InputUpdate()
{
	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		DestroyWindow(m_winCore->GetHWND());
}
