#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <dwmapi.h>
#include <memory>
#include <string>
#include <chrono>
#include <engine/sdk/math/vector.hpp>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <renderer/renderer.hpp>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dwmapi.lib")

class D3D11Renderer : public Renderer {
public:
    D3D11Renderer();
    ~D3D11Renderer();

    bool initialize() override;
    void shutdown() override;
    bool is_initialized() const override { return m_initialized; }

    void begin_frame() override;
    void end_frame() override;
    void present() override;

    void begin_imgui_frame() override;
    void end_imgui_frame() override;
    void render_imgui() override;

    void draw_line(const Vector2& start, const Vector2& end, const ImColor& color, float thickness = 1.0f) override;
    void draw_rect(const Vector2& position, const Vector2& size, const ImColor& color, float thickness = 1.0f) override;
    void draw_filled_rect(const Vector2& position, const Vector2& size, const ImColor& color) override;
    void draw_circle(const Vector2& center, float radius, const ImColor& color, int segments = 32, float thickness = 1.0f) override;
    void draw_filled_circle(const Vector2& center, float radius, const ImColor& color, int segments = 32) override;
    void draw_text(const Vector2& position, const std::string& text, const ImColor& color, float size = 16.0f) override;

    Vector2 get_screen_size() const override { return m_screen_size; }
    Vector2 get_screen_center() const override { return m_screen_size / 2; }
    float get_framerate() const override { return m_current_fps; }

    ID3D11Device* get_device() const { return m_device; }
    ID3D11DeviceContext* get_context() const { return m_context; }
    HWND get_window() const { return m_overlay_window; }

    void process_input();

private:
    std::wstring m_overlay_class_name;
    bool m_initialized;
    bool m_imgui_initialized;
    HWND m_overlay_window;
    Vector2 m_screen_size;
    UINT m_refresh_rate;

    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    IDXGISwapChain* m_swap_chain;
    ID3D11RenderTargetView* m_render_target_view;

    float m_current_fps;
    float m_frame_time_accumulator;
    int m_frame_count;
    std::chrono::high_resolution_clock::time_point m_last_time;

    bool create_overlay_window();
    bool create_device_and_swap_chain();
    bool create_render_target();
    bool initialize_imgui();
    void cleanup_imgui();
    void cleanup_d3d_objects();
};