#include "Application.h"

#include "../GOE_Core/Core_pch.h"
#include "../GOE_Render/Renderer_pch.h"
#include "../GOE_Editor/Editor_pch.h"
#include "../GOE_AssetLoader/AssetLoader_pch.h"

#include "../GOE_Editor/DebugManager.h"
#include "../GOE_Render/GOERenderer.h"
#include "../GOE_Editor/EditorCore.h"
#include "../GOE_Core/Window.h"
#include "../GOE_AssetLoader/AssetCore.h"


Application::Application(HINSTANCE hInst, int nCmdShow)
	:m_winCore(nullptr), m_renderer(nullptr), m_editor(nullptr)
{
	m_winCore = std::make_unique<Window>(L"GOE", 1200, 800, hInst, nCmdShow);
}

Application::~Application(){}

void Application::Initialize()
{
	m_winCore->InitInstance();
	m_winCore->SetExternalMsgHandler(&ImGui_ImplWin32_WndProcHandler);

	m_assetCore = std::make_unique<AssetCore>();
	m_assetCore.get()->CreateAssetLoader();
	m_assetCore.get()->LoadModel("D:\\project\\GOE\\GOE\\Assets\\models\\Ch03_nonPBR.fbx");
	
	m_renderer = std::make_unique<GOERenderer>(m_winCore->GetHWND()); 
	m_renderer->OnInit();
	m_renderer->CreateAllModelResource(m_assetCore.get()->GetModels());
	m_renderer->CopyUploadHeapToDefault();
	
	//m_renderer->TransVertexuramon();
	m_editor = std::make_unique<EditorCore>(m_winCore->GetHWND());
	m_editor->Initialize(m_renderer.get()->GetUIInfo());	
}

int Application::Run()
{
	while (m_winCore->ProcessMessages())
	{
		m_editor->OnUpdate();
		m_renderer->OnUpdate();
		DebugManager::GetInstance().OnDebugUpdate();

		m_renderer->BeginRender();
		m_renderer->OnRender(); // 렌더링 호출
		m_editor->OnRender(m_renderer.get()->GetUILoopInfo());
		m_renderer->EndRender();

		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) DestroyWindow(m_winCore->GetHWND());
	}

	return 0;
}



