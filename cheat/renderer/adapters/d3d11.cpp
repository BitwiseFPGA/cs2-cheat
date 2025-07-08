#include <renderer/adapters/d3d11.hpp>
#include <logger/logger.hpp>
#include <config/runtime/settings.hpp>
#include <iostream>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return true;

    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        return 0;
    case WM_SYSCOMMAND:
        if ((wparam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

D3D11Renderer::D3D11Renderer()
    : m_overlay_class_name(L"template-overlay")
    , m_initialized(false)
    , m_imgui_initialized(false)
    , m_overlay_window(nullptr)
    , m_screen_size(1920.0f, 1080.0f)
    , m_refresh_rate(60)
    , m_device(nullptr)
    , m_context(nullptr)
    , m_swap_chain(nullptr)
    , m_render_target_view(nullptr)
    , m_current_fps(0.0f)
    , m_frame_time_accumulator(0.0f)
    , m_frame_count(0)
    , m_last_time(std::chrono::high_resolution_clock::now())
    , m_draw_list(nullptr)
{
}

D3D11Renderer::~D3D11Renderer() {
    if (m_initialized) {
        shutdown();
    }
    logger::info("D3D11 renderer destructor called");
}

bool D3D11Renderer::initialize() {
    logger::log_step("D3D11 Renderer Init", "Setting up ImGui overlay");

    try {
        if (!create_overlay_window()) {
            logger::log_failure("D3D11 Renderer", "Failed to create overlay window");
            return false;
        }

        if (!create_device_and_swap_chain()) {
            logger::log_failure("D3D11 Renderer", "Failed to create D3D11 device");
            return false;
        }

        if (!create_render_target()) {
            logger::log_failure("D3D11 Renderer", "Failed to create render target");
            return false;
        }

        if (!initialize_imgui()) {
            logger::log_failure("D3D11 Renderer", "Failed to initialize ImGui");
            return false;
        }

        m_initialized = true;

        return true;

    }
    catch (const std::exception& e) {
        logger::log_failure("D3D11 Renderer", e.what());
        cleanup_d3d_objects();
        return false;
    }
}

void D3D11Renderer::shutdown() {
    logger::info("Shutting down D3D11 renderer");

    cleanup_imgui();
    cleanup_d3d_objects();
    m_initialized = false;
    m_imgui_initialized = false;

    logger::info("D3D11 renderer shutdown completed");
}

bool D3D11Renderer::create_overlay_window() {
    try {
        int m_i_width = GetSystemMetrics(SM_CXSCREEN);
        int m_i_height = GetSystemMetrics(SM_CYSCREEN);

        m_screen_size.x = static_cast<float>(m_i_width);
        m_screen_size.y = static_cast<float>(m_i_height);

        WNDCLASSEXW window_class = {};
        window_class.cbSize = sizeof(WNDCLASSEXW);
        window_class.style = 0;
        window_class.lpfnWndProc = OverlayWndProc;
        window_class.hInstance = GetModuleHandle(nullptr);
        window_class.lpszClassName = m_overlay_class_name.c_str();

        if (!RegisterClassExW(&window_class)) {
            DWORD error = GetLastError();
            if (error != ERROR_CLASS_ALREADY_EXISTS) {
                logger::error("Failed to register window class. Error: " + std::to_string(error));
                return false;
            }
        }

        m_overlay_window = CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
            window_class.lpszClassName,
            L"",
            WS_POPUP | WS_VISIBLE,
            0, 0, m_i_width, m_i_height,
            nullptr, nullptr, GetModuleHandle(nullptr), nullptr
        );

        if (!m_overlay_window) {
            logger::error("Failed to create overlay window. Error: " + std::to_string(GetLastError()));
            return false;
        }

        if (!SetLayeredWindowAttributes(m_overlay_window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA)) {
            logger::error("Failed to set layered window attributes");
            return false;
        }

        {
            RECT clientArea = {};
            if (!GetClientRect(m_overlay_window, &clientArea)) {
                logger::error("Failed to get client rect");
                return false;
            }

            RECT windowArea = {};
            if (!GetWindowRect(m_overlay_window, &windowArea)) {
                logger::error("Failed to get window rect");
                return false;
            }

            POINT diff = {};
            if (!ClientToScreen(m_overlay_window, &diff)) {
                logger::error("Failed to get client to screen");
                return false;
            }

            const MARGINS margins{
                windowArea.left + (diff.x - windowArea.left),
                windowArea.top + (diff.y - windowArea.top),
                windowArea.right,
                windowArea.bottom
            };

            if (FAILED(DwmExtendFrameIntoClientArea(m_overlay_window, &margins))) {
                logger::error("Failed to extend frame into client area");
                return false;
            }
        }

        HDC hDC = GetDC(m_overlay_window);
        m_refresh_rate = GetDeviceCaps(hDC, VREFRESH);
        ReleaseDC(m_overlay_window, hDC);

        SetWindowLong(m_overlay_window, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
        ShowWindow(m_overlay_window, SW_SHOW);
        UpdateWindow(m_overlay_window);

        logger::debug("Transparent overlay window created successfully (" + std::to_string(m_i_width) + "x" + std::to_string(m_i_height) + ") @ " + std::to_string(m_refresh_rate) + "Hz");
        return true;

    }
    catch (const std::exception& e) {
        logger::error("Exception in create_overlay_window: " + std::string(e.what()));
        return false;
    }
}

bool D3D11Renderer::create_device_and_swap_chain() {
    try {
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferDesc.RefreshRate.Numerator = m_refresh_rate;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1U;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.SampleDesc.Count = 1U;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2U;
        swapChainDesc.OutputWindow = m_overlay_window;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        constexpr D3D_FEATURE_LEVEL levels[2]{
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_0
        };

        D3D_FEATURE_LEVEL level = {};

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0U,
            levels,
            2U,
            D3D11_SDK_VERSION,
            &swapChainDesc,
            &m_swap_chain,
            &m_device,
            &level,
            &m_context
        );

        if (hr != S_OK) {
            logger::error("Failed to create D3D11 device and swap chain. HRESULT: " + std::to_string(hr));
            return false;
        }

        logger::debug("D3D11 device and swap chain created successfully with " + std::to_string(m_refresh_rate) + "Hz refresh rate");
        return true;

    }
    catch (const std::exception& e) {
        logger::error("Exception in create_device_and_swap_chain: " + std::string(e.what()));
        return false;
    }
}

bool D3D11Renderer::create_render_target() {
    try {
        ID3D11Texture2D* pBackBuffer = nullptr;
        HRESULT hr = m_swap_chain->GetBuffer(0U, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

        if (FAILED(hr)) {
            logger::error("Failed to get back buffer. HRESULT: " + std::to_string(hr));
            return false;
        }

        if (pBackBuffer) {
            hr = m_device->CreateRenderTargetView(pBackBuffer, nullptr, &m_render_target_view);
            pBackBuffer->Release();

            if (FAILED(hr)) {
                logger::error("Failed to create render target view. HRESULT: " + std::to_string(hr));
                return false;
            }
        }
        else {
            logger::error("Failed to get back buffer - buffer is null");
            return false;
        }

        logger::debug("Render target created successfully");
        return true;

    }
    catch (const std::exception& e) {
        logger::error("Exception in create_render_target: " + std::string(e.what()));
        return false;
    }
}

bool D3D11Renderer::initialize_imgui() {
    logger::log_step("ImGui Init", "Setting up ImGui context and backends");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 1.0f;
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.Alpha = 0.9f;

    if (!ImGui_ImplWin32_Init(m_overlay_window)) {
        logger::error("Failed to initialize ImGui Win32 backend");
        return false;
    }

    if (!ImGui_ImplDX11_Init(m_device, m_context)) {
        logger::error("Failed to initialize ImGui DX11 backend");
        ImGui_ImplWin32_Shutdown();
        return false;
    }

    m_imgui_initialized = true;
    logger::debug("ImGui initialized successfully");
    return true;
}

void D3D11Renderer::cleanup_imgui() {
    if (m_imgui_initialized) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        m_imgui_initialized = false;
    }
}

void D3D11Renderer::cleanup_d3d_objects() {
    if (m_render_target_view) { m_render_target_view->Release(); m_render_target_view = nullptr; }
    if (m_swap_chain) { m_swap_chain->Release(); m_swap_chain = nullptr; }
    if (m_context) { m_context->Release(); m_context = nullptr; }
    if (m_device) { m_device->Release(); m_device = nullptr; }

    if (m_overlay_window) {
        DestroyWindow(m_overlay_window);
        m_overlay_window = nullptr;
    }

    UnregisterClassW(m_overlay_class_name.c_str(), GetModuleHandle(nullptr));
}

void D3D11Renderer::begin_frame() {
    if (!m_initialized) return;

    process_input();

    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_context->OMSetRenderTargets(1, &m_render_target_view, nullptr);
    m_context->ClearRenderTargetView(m_render_target_view, clear_color);
}

void D3D11Renderer::end_frame() {
    if (!m_initialized) return;
}

void D3D11Renderer::present() {
    if (!m_initialized) return;
    
    auto current_time = std::chrono::high_resolution_clock::now();
    auto delta_time = std::chrono::duration<float>(current_time - m_last_time).count();
    m_last_time = current_time;

    m_frame_time_accumulator += delta_time;
    m_frame_count++;

    if (m_frame_time_accumulator >= 1.0f) {
        m_current_fps = static_cast<float>(m_frame_count) / m_frame_time_accumulator;
        m_frame_time_accumulator = 0.0f;
        m_frame_count = 0;
    }
    
    m_swap_chain->Present(0, 0);
}

void D3D11Renderer::begin_imgui_frame() {
    if (!m_imgui_initialized) return;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    m_draw_list = ImGui::GetBackgroundDrawList();
}

void D3D11Renderer::end_imgui_frame() {
    if (!m_imgui_initialized) return;

    ImGui::Render();

    m_context->OMSetRenderTargets(1, &m_render_target_view, nullptr);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void D3D11Renderer::render_imgui() {
    if (!m_imgui_initialized) return;
}

void D3D11Renderer::draw_line(const Vector2& start, const Vector2& end, const ImColor& color, float thickness) {
    if (!m_imgui_initialized) return;

    m_draw_list->AddLine(
        ImVec2(start.x, start.y),
        ImVec2(end.x, end.y),
        color,
        thickness
    );
}

void D3D11Renderer::draw_rect(const Vector2& position, const Vector2& size, const ImColor& color, float thickness) {
    if (!m_imgui_initialized) return;

    m_draw_list->AddRect(
        ImVec2(position.x, position.y),
        ImVec2(position.x + size.x, position.y + size.y),
        color,
        0.0f,
        0,
        thickness
    );
}

void D3D11Renderer::draw_filled_rect(const Vector2& position, const Vector2& size, const ImColor& color) {
    if (!m_imgui_initialized) return;

    m_draw_list->AddRectFilled(
        ImVec2(position.x, position.y),
        ImVec2(position.x + size.x, position.y + size.y),
        color
    );
}

void D3D11Renderer::draw_circle(const Vector2& center, float radius, const ImColor& color, int segments, float thickness) {
    if (!m_imgui_initialized) return;

    m_draw_list->AddCircle(
        ImVec2(center.x, center.y),
        radius,
        color,
        segments,
        thickness
    );
}

void D3D11Renderer::draw_filled_circle(const Vector2& center, float radius, const ImColor& color, int segments) {
    if (!m_imgui_initialized) return;

    m_draw_list->AddCircleFilled(
        ImVec2(center.x, center.y),
        radius,
        color,
        segments
    );
}

void D3D11Renderer::draw_text(const Vector2& position, const std::string& text, const ImColor& color, float size) {
    if (!m_imgui_initialized) return;

    ImFont* font = ImGui::GetFont();

    m_draw_list->AddText(
        ImVec2(position.x, position.y),
        color,
        text.c_str()
    );
}

void D3D11Renderer::process_input() {
    if (!m_initialized) return;

    LONG_PTR windowStyle = GetWindowLongPtr(m_overlay_window, GWL_EXSTYLE);
    if (!settings::g_show_menu) {
        SetWindowLongPtr(m_overlay_window, GWL_EXSTYLE, windowStyle | WS_EX_TRANSPARENT);
    }
    else {
        SetWindowLongPtr(m_overlay_window, GWL_EXSTYLE, windowStyle & ~WS_EX_TRANSPARENT);
        SetForegroundWindow(m_overlay_window);
    }

    if (GetAsyncKeyState(VK_END) & 1) {
        logger::info("END key pressed - shutting down application");
        exit(0);
    }
}