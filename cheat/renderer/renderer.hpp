#pragma once
#include <string>
#include <engine/sdk/math/vector.hpp>

class Renderer {
public:
    virtual ~Renderer() = default;
    
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual bool is_initialized() const = 0;

    virtual void begin_frame() = 0;
    virtual void end_frame() = 0;
    virtual void present() = 0;

    virtual void begin_imgui_frame() = 0;
    virtual void end_imgui_frame() = 0;
    virtual void render_imgui() = 0;

    virtual void draw_line(const Vector2& start, const Vector2& end, const ImColor& color, float thickness = 1.0f) = 0;
    virtual void draw_rect(const Vector2& position, const Vector2& size, const ImColor& color, float thickness = 1.0f) = 0;
    virtual void draw_filled_rect(const Vector2& position, const Vector2& size, const ImColor& color) = 0;
    virtual void draw_circle(const Vector2& center, float radius, const ImColor& color, int segments = 32, float thickness = 1.0f) = 0;
    virtual void draw_filled_circle(const Vector2& center, float radius, const ImColor& color, int segments = 32) = 0;
    virtual void draw_text(const Vector2& position, const std::string& text, const ImColor& color, float size = 16.0f) = 0;

    virtual Vector2 get_screen_size() const = 0;
    virtual Vector2 get_screen_center() const = 0;
    virtual float get_framerate() const = 0;
};
