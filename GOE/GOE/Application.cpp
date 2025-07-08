#include "Application.h"
#include "../GOERender/GOERenderer.h"  
#include "../Imgui/imgui_impl_dx12.h"
#include "../Imgui/imgui_impl_win32.h"
// Simple free list based allocator

static ExampleDescriptorHeapAllocator g_pd3dSrvDescHeapAlloc;


Application::Application(GOE::WinDesc info)
	: m_hInst(info.hInstance), m_nCmdShow(info.nCmdShow),
	m_name(info.name), m_windowSize({ static_cast<SHORT>(info.width), static_cast<SHORT>(info.height) }),
	m_renderer(nullptr), m_hWnd(nullptr), m_msg({})
{
}

Application::~Application()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	delete m_allocator;
}
ImGuiIO g_io;
void Application::Initialize()
{
	InitInstance(); // 인스턴스 초기화 함수 호출
	/// 나중에 엔진으로 편입될예정 반드시 지워야해
	m_renderer = std::make_unique<GOERenderer>(m_hWnd); // GOERenderer 인스턴스 생성
	m_renderer->OnInit(); // GOERenderer 초기화
	g_pd3dSrvDescHeapAlloc.Create(dynamic_cast<GOERenderer*>(m_renderer.get())->GetDevice(),
		dynamic_cast<GOERenderer*>(m_renderer.get())->GetDescriptorHeap());
	/// imgui test
	{
		// Make process DPI aware and obtain main monitor scale
// 모니터 배율을 가져온다.
		ImGui_ImplWin32_EnableDpiAwareness();
		// 원래는 이걸 화면비에 곱해주는데 지금은 그런거 없다.
		float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
		
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		g_io = io;
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


		
		// Setup Platform/Renderer backends
		ImGui_ImplDX12_InitInfo init_info = {};
		init_info.Device = dynamic_cast<GOERenderer*>(m_renderer.get())->GetDevice();
		init_info.CommandQueue = dynamic_cast<GOERenderer*>(m_renderer.get())->GetCommandQueue();
		init_info.NumFramesInFlight = dynamic_cast<GOERenderer*>(m_renderer.get())->GetFrameCount();
		init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // Or your render target format.

		// Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
		// The example_win32_directx12/main.cpp application include a simple free-list based allocator.
		init_info.SrvDescriptorHeap = dynamic_cast<GOERenderer*>(m_renderer.get())->GetDescriptorHeap();
		init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle)
			{
				return g_pd3dSrvDescHeapAlloc.Alloc(out_cpu_handle, out_gpu_handle); /*YOUR_ALLOCATOR_FUNCTION_FOR_SRV_DESCRIPTORS(...)*/;
			};
		init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)
			{
				return g_pd3dSrvDescHeapAlloc.Free(cpu_handle, gpu_handle);
			};

		// (before 1.91.6 the DirectX12 backend required a single SRV descriptor passed)
		// (there is a legacy version of ImGui_ImplDX12_Init() that supports those, but a future version of Dear ImGuii will requires more descriptors to be allocated)

		ImGui_ImplDX12_Init(&init_info);
	}
}

int Application::Run()
{
	while (true)
	{
		// 메시지가 있으면 처리
		if (PeekMessage(&m_msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (m_msg.message == WM_QUIT)
				break;

			TranslateMessage(&m_msg);
			DispatchMessage(&m_msg);
		}
		
		///Imgui테스트를 위한 구간

		
		// (Your code process and dispatch Win32 messages)
		// Start the Dear ImGui frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);
		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		{

			
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / g_io.Framerate, g_io.Framerate);
			ImGui::End();

		}
		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		m_renderer->OnRender(); // 렌더링 호출


		
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) DestroyWindow(m_hWnd);
	}

	return 0;
}

ATOM Application::MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;                       // 윈도우 클래스 정보를 담는 구조체 (확장된 버전)
	wcex.cbSize = sizeof(WNDCLASSEX);       // 구조체의 크기 설정 (필수)
	wcex.style = CS_HREDRAW | CS_VREDRAW;   // 창 크기 변경 시 전체를 다시 그리도록 설정
	wcex.lpfnWndProc = WndProc;             // 윈도우 메시지를 처리할 콜백 함수 (윈도우 프로시저)
	wcex.cbClsExtra = 0;                    // 클래스 여분 메모리 (사용하지 않으면 0)
	wcex.cbWndExtra = 0;                    // 윈도우 인스턴스 여분 메모리 (사용하지 않으면 0)
	wcex.hInstance = hInstance;             // 애플리케이션 인스턴스 핸들
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GOE)); // 큰 아이콘 로드S
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);      // 기본 마우스 커서 지정 (화살표)
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);    // 배경 브러시 (기본 창 배경색)
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GOE);      // 리소스에 정의된 메뉴 이름 지정
	wcex.lpszClassName = m_name.c_str();                 // 이 윈도우 클래스의 이름
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL)); // 작은 아이콘 (타이틀 바에 표시)
	return RegisterClassExW(&wcex);                     // 설정한 윈도우 클래스를 운영체제에 등록
}

BOOL Application::InitInstance()
{
	MyRegisterClass(m_hInst);

	m_hWnd = CreateWindowW(
		m_name.c_str(),			// 윈도우 클래스 이름
		m_name.c_str(),			// 윈도우 제목
		WS_OVERLAPPEDWINDOW,	// 윈도우 스타일 (오버랩된 창)
		CW_USEDEFAULT,			// x 위치 (기본값)
		0,						// y 위치 (기본값)
		m_windowSize.X,			// 너비 (기본값)
		m_windowSize.Y,			// 높이 (기본값)
		nullptr,				// 부모 윈도우 핸들 (없으면 nullptr)
		nullptr,				// 메뉴 핸들 (없으면 nullptr)
		m_hInst,				// 애플리케이션 인스턴스 핸들
		nullptr);				// 추가 매개변수 (없으면 nullptr)

	if (!m_hWnd)
	{
		return FALSE;
	}

	ShowWindow(m_hWnd, m_nCmdShow);
	UpdateWindow(m_hWnd);

	return TRUE;
}



LRESULT Application::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;
	switch (message)
	{

		case WM_DESTROY:
			PostQuitMessage(0); // 애플리케이션 종료 요청
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam); // 기본 처리 위임
	}

	return 0;
}


