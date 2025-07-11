#include "EditorCore.h"

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
	Editor::g_io = io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

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


void EditorCore::OnUpdate()
{
	///Imgui테스트를 위한 구간
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	static float f = 0.0f;
	static int counter = 0;

	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / Editor::g_io.Framerate, Editor::g_io.Framerate);
	ImGui::End();
}

void EditorCore::OnRender(UILoopInfo* uiInfo)
{
	ImGui::Render();

	D3D12_RESOURCE_BARRIER imguiBarrier = {};
	imguiBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	imguiBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	imguiBarrier.Transition.pResource = uiInfo->rendertarget;
	imguiBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	imguiBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	imguiBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	uiInfo->commandlist->ResourceBarrier(1, &imguiBarrier);

	ID3D12DescriptorHeap* pHeap = uiInfo->imguiDescriptorHeap;

	// 임시 변수의 주소를 전달합니다.
	uiInfo->commandlist->SetDescriptorHeaps(1, &pHeap);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), uiInfo->commandlist);
	imguiBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	imguiBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	uiInfo->commandlist->ResourceBarrier(1, &imguiBarrier);

}