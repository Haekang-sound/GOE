#include "Editor_pch.h"
#include "EditorCore.h"
#include "DebugManager.h"

namespace Editor
{
	ImGuiIO g_io;
	ExampleDescriptorHeapAllocator g_allocator;
}

EditorCore::EditorCore(HWND hwnd)
	:m_hWnd(hwnd)
{
}

EditorCore::EditorCore()
{
}


EditorCore::~EditorCore()
{
	Editor::g_allocator.Destroy();

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


void EditorCore::Initialize(UIInitInfo* uiInfo)
{
	ImGui_ImplWin32_EnableDpiAwareness();
	float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	/// 폰트설정
	{
		const char* font_path = "..\\Fonts\\D2Coding.ttc"; // .ttc 파일 경로
		float font_size = 16.0f;

		// 한글 범위 지정
		static const ImWchar korean_ranges[] =
		{
			0x0020, 0x007E, // Basic Latin
			0x3131, 0x3163, // Hangul Jamo
			0xAC00, 0xD7A3, // Hangul Syllables
			0,
		};

		// 1. 폰트 설정을 위한 객체 생성
		ImFontConfig font_config;

		// 2. .ttc 파일 내에서 사용할 폰트의 인덱스를 지정 (0 = 첫 번째 폰트)
		// D2Coding.ttc의 경우 보통 0번이 일반(Regular) 굵기입니다.
		font_config.FontNo = 0;

		// 3. 설정 객체와 함께 폰트 로드
		io.Fonts->AddFontFromFileTTF(font_path, font_size, &font_config, korean_ranges);

		// 폰트 텍스처 빌드는 백엔드가 자동으로 처리해줍니다.
	}

	Editor::g_io = io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

	ImGui_ImplWin32_Init(m_hWnd);

	Editor::g_allocator.Create(uiInfo->device, uiInfo->imguiDescriptorHeap);

	// Setup Platform/Renderer backends
	ImGui_ImplDX12_InitInfo init_info = {};
	init_info.Device = uiInfo->device;
	init_info.CommandQueue = uiInfo->commandQueue;
	init_info.NumFramesInFlight = uiInfo->frameBufferCount;
	init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // Or your render target format.

	// Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
	// The example_win32_directx12/main.cpp application include a simple free-list based allocator.
	init_info.SrvDescriptorHeap = uiInfo->imguiDescriptorHeap;
	init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
		{
			return Editor::g_allocator.Alloc(out_cpu_handle, out_gpu_handle); /*YOUR_ALLOCATOR_FUNCTION_FOR_SRV_DESCRIPTORS(...)*/;
		};
	init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
		{
			return Editor::g_allocator.Free(cpu_handle, gpu_handle);
		};

	ImGui_ImplDX12_Init(&init_info);
}


void EditorCore::OnUpdate(double dTime)
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void EditorCore::OnRender(UILoopInfo* uiInfo)
{
	ImGui::Render();
	ID3D12DescriptorHeap* pHeap = uiInfo->imguiDescriptorHeap;
	uiInfo->commandlist->SetDescriptorHeaps(1, &pHeap);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), uiInfo->commandlist);
}