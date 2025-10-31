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

	///애니메이션 로드
	m_assetCore.get()->LoadAnimation("D:\\project\\GOE\\GOE\\Assets\\animation\\Samba Dancing.fbx");

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
/// 임시
// 애니메이션 재생시간(animTime)에 따라
// 현재 사용해야 할 키프레임 인덱스를 구하는 함수
int GetFrameIndexFromTime(float animTime, float duration, int frameCount)
{
	// 1️⃣ 방어 코드
	// 프레임이 1개 이하거나, duration이 0이면 무조건 0번째 프레임
	if (frameCount <= 1 || duration <= 0.0f)
	{
		return 0;
	}

	// 2️⃣ 현재 애니메이션의 진행률을 구함 (0.0 ~ 1.0)
	// animTime은 tick 단위, duration도 tick 단위이므로 단순 비율 계산 가능
	float normalizedTime = animTime / duration;

	// 3️⃣ 1.0을 넘어가면 루프 애니메이션이므로 다시 0~1로 돌림
	// (fmod를 사용하면 animTime이 duration을 초과해도 정상 반복)
	normalizedTime = fmod(normalizedTime, 1.0f);

	// 4️⃣ 음수 보정 (혹시 animTime이 음수가 될 경우 대비)
	if (normalizedTime < 0.0f)
	{
		normalizedTime += 1.0f;
	}

	// 5️⃣ 진행률에 전체 프레임 개수를 곱해서 실제 인덱스로 변환
	float frameFloat = normalizedTime * frameCount;

	// 6️⃣ 실수를 정수 인덱스로 변환
	int frameIndex = static_cast<int>(frameFloat);

	// 7️⃣ 마지막 프레임을 넘어가지 않게 보정
	if (frameIndex >= frameCount)
	{
		frameIndex = frameCount - 1;
	}

	// 8️⃣ 계산된 프레임 인덱스를 반환
	return frameIndex;
}
void GOE::Engine::OnUpdate(double dTime)
{
	InputUpdate();

	m_sceneManager.get()->OnUpdate(dTime);

	m_editor->OnUpdate();
	m_renderer->OnUpdate();


#pragma region testAnimation

	/// 값이 정확한지는 아직 모르지만 애니메이션 업데이트는 구조적으로 만들었다.
	//1. 애니메이션-> 노드
	// 특정 애니매이션을 id로 가져온다.
	/// 애니메이션을 해쉬로저장할떄 단순 이름으로 저장하면 안될것 같다 왜냐면 애니메이션 이름이 죄다 mixamo.com 이기때문에
	Animation* temp = m_assetCore->GetAnimation(GOE::FileManager::GetHash("mixamo.com"));
	// 모델을 가져온다
	Model* tempM = m_assetCore->GetModel((GOE::FileManager::GetHash("D:\\project\\GOE\\GOE\\Assets\\models\\Ch03_nonPBR.fbx")));

	static float elapsedTime = 0.0f;
	elapsedTime += (float)dTime;

	float ticksPerSecond = temp->GetTicksPerSecond(); // 예: 30
	float duration = temp->GetDuration();              // 예: 559

	float timeInTicks = elapsedTime * ticksPerSecond; // 초 → tick 변환
	float animTime = fmod(timeInTicks, duration);     // 루프 재생
	int keyframeCount = 155; // 예: 155
	int frameIndex = GetFrameIndexFromTime(animTime, duration, keyframeCount);

	// 1. 애니메이션은 순회하면서 모델 내부의 노드를 업데이트한다.
	for (int i = 0; i < temp->GetBoneAnimation().size(); ++i)
	{
		BoneAnimation* boneAnim = temp->GetBoneAnimation()[i].get();
		Node* currentNode = tempM->GetNodeFromMap(boneAnim->GetID());
		if (currentNode)
		{ 
			currentNode->SetLocalTM(boneAnim->GetSRTMatrix(frameIndex) * currentNode->GetNodePositionMatrix());
		}
		int c = 3;
	}

	/// 2. 모델의 노드 계층구조를 업데이트한다.
	tempM->UpdateNodeHierarchy();

	static int tmIndex = 0;

	static bool prevT = false;
	bool l = (GetAsyncKeyState('R') & 0x8000);
	bool r = (GetAsyncKeyState('T') & 0x8000);

	bool nowT = l || r;

	size_t boneAnimCount = temp->GetBoneAnimation().size();
	if (nowT && !prevT)
	{
		// 키 눌린 "순간" 한 번만 실행

		// [수정] 기준을 모델의 전체 노드 개수가 아닌, 애니메이션의 뼈 개수로 변경합니다.
		if (boneAnimCount > 0)
		{
			if (r)
			{
				// tmIndex를 boneAnimCount 기준으로 안전하게 순환시킵니다.
				tmIndex = (tmIndex + 1) % boneAnimCount;
			}
			else
			{
				tmIndex -= 1;
				if (tmIndex < 0) tmIndex = 51;
				tmIndex = (tmIndex) % boneAnimCount;
			}

		}
	}

	prevT = nowT;


	DebugManager::GetInstance().PushDebugData(
		[&]()
		{
			// ImGui를 그리기 전에 tmIndex가 유효한 범위 내에 있는지 확인합니다.
			if (tmIndex < temp->GetBoneAnimation().size())
			{
				// BoneAnimation에서 ID를 가져옵니다.
				size_t boneAnimID = temp->GetBoneAnimation()[tmIndex].get()->GetID();

				// 해당 ID로 모델에서 노드를 찾습니다.
				Node* targetNode = tempM->GetNodeFromMap(boneAnimID);

				// 노드를 성공적으로 찾았는지 확인합니다.
				while (!targetNode)
				{
					if (r)
						tmIndex = (tmIndex + 1) % boneAnimCount;
					else
					{
						tmIndex -= 1;
						if (tmIndex < 0) tmIndex = 51;
						tmIndex = (tmIndex) % boneAnimCount;
					}

					// BoneAnimation에서 ID를 가져옵니다.
					boneAnimID = temp->GetBoneAnimation()[tmIndex].get()->GetID();

					// 해당 ID로 모델에서 노드를 찾습니다.
					targetNode = tempM->GetNodeFromMap(boneAnimID);
				}

				{
					ImGui::Begin("testAnimation");
					ImGui::Text("조작 : R, T");
					ImGui::Text("Node Name: %s (Index: %zu)", targetNode->GetName().c_str(), tmIndex);
					ImGui::Text("%.6f, %.6f, %.6f, %.6f", targetNode->GetWorldTM()._11, targetNode->GetWorldTM()._12, targetNode->GetWorldTM()._13, targetNode->GetWorldTM()._14);
					ImGui::Text("%.6f, %.6f, %.6f, %.6f", targetNode->GetWorldTM()._21, targetNode->GetWorldTM()._22, targetNode->GetWorldTM()._23, targetNode->GetWorldTM()._24);
					ImGui::Text("%.6f, %.6f, %.6f, %.6f", targetNode->GetWorldTM()._31, targetNode->GetWorldTM()._32, targetNode->GetWorldTM()._33, targetNode->GetWorldTM()._34);
					ImGui::Text("%.6f, %.6f, %.6f, %.6f\n", targetNode->GetWorldTM()._41, targetNode->GetWorldTM()._42, targetNode->GetWorldTM()._43, targetNode->GetWorldTM()._44);

					ImGui::Text("");
					ImGui::Text("Node local Name: %s (Index: %zu)", targetNode->GetName().c_str(), tmIndex);
					ImGui::Text("%.6f, %.6f, %.6f, %.6f", targetNode->GetLocalTM()._11, targetNode->GetLocalTM()._12, targetNode->GetLocalTM()._13, targetNode->GetLocalTM()._14);
					ImGui::Text("%.6f, %.6f, %.6f, %.6f", targetNode->GetLocalTM()._21, targetNode->GetLocalTM()._22, targetNode->GetLocalTM()._23, targetNode->GetLocalTM()._24);
					ImGui::Text("%.6f, %.6f, %.6f, %.6f", targetNode->GetLocalTM()._31, targetNode->GetLocalTM()._32, targetNode->GetLocalTM()._33, targetNode->GetLocalTM()._34);
					ImGui::Text("%.6f, %.6f, %.6f, %.6f\n", targetNode->GetLocalTM()._41, targetNode->GetLocalTM()._42, targetNode->GetLocalTM()._43, targetNode->GetLocalTM()._44);

					ImGui::Text("");
					ImGui::Text("bone anim: %s (ID: %zu)", temp->GetBoneAnimation()[0].get()->GetName(), temp->GetBoneAnimation()[0].get()->GetID());
					ImGui::Text("%.6f, %.6f, %.6f, %.6f", temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._11, temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._12, temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._13, temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._14);
					ImGui::Text("%.6f, %.6f, %.6f, %.6f", temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._21, temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._22, temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._23, temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._24);
					ImGui::Text("%.6f, %.6f, %.6f, %.6f", temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._31, temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._32, temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._33, temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._34);
					ImGui::Text("%.6f, %.6f, %.6f, %.6f\n", temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._41, temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._42, temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._43, temp->GetBoneAnimation()[0].get()->GetSRTMatrix(0)._44);

					ImGui::End();
				}
			}
		});

#pragma endregion

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
void GOE::Engine::Release() {}


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
