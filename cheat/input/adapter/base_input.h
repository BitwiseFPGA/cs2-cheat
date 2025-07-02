#pragma once
#include <string>
#include <cstdint>
#include <windows.h>

// Virtual key codes for consistent usage across adapters
enum class InputKey : uint16_t {
    None = 0,
    MouseLeft = VK_LBUTTON,
    MouseRight = VK_RBUTTON,
    MouseMiddle = VK_MBUTTON,
    MouseX1 = VK_XBUTTON1,
    MouseX2 = VK_XBUTTON2,
    
    Backspace = VK_BACK,
    Tab = VK_TAB,
    Enter = VK_RETURN,
    Shift = VK_SHIFT,
    Ctrl = VK_CONTROL,
    Alt = VK_MENU,
    Escape = VK_ESCAPE,
    Space = VK_SPACE,
    Insert = VK_INSERT,
    
    Left = VK_LEFT,
    Up = VK_UP,
    Right = VK_RIGHT,
    Down = VK_DOWN,
    
    A = 0x41, B = 0x42, C = 0x43, D = 0x44, E = 0x45, F = 0x46,
    G = 0x47, H = 0x48, I = 0x49, J = 0x4A, K = 0x4B, L = 0x4C,
    M = 0x4D, N = 0x4E, O = 0x4F, P = 0x50, Q = 0x51, R = 0x52,
    S = 0x53, T = 0x54, U = 0x55, V = 0x56, W = 0x57, X = 0x58,
    Y = 0x59, Z = 0x5A,
    
    Num0 = 0x30, Num1 = 0x31, Num2 = 0x32, Num3 = 0x33, Num4 = 0x34,
    Num5 = 0x35, Num6 = 0x36, Num7 = 0x37, Num8 = 0x38, Num9 = 0x39,
    
    F1 = VK_F1, F2 = VK_F2, F3 = VK_F3, F4 = VK_F4, F5 = VK_F5, F6 = VK_F6,
    F7 = VK_F7, F8 = VK_F8, F9 = VK_F9, F10 = VK_F10, F11 = VK_F11, F12 = VK_F12
};

struct MousePosition {
    int x;
    int y;
};

enum class MouseButton : uint8_t {
    Left = 0,
    Right = 1,
    Middle = 2,
    X1 = 3,
    X2 = 4
};

class BaseInputAdapter {
public:
    BaseInputAdapter();
    virtual ~BaseInputAdapter();

    // Initialization and cleanup
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    // Key state queries
    virtual bool is_key_down(InputKey key) = 0;
    virtual bool is_key_up(InputKey key) = 0;
    virtual bool is_key_pressed(InputKey key) = 0; // Single press detection
    virtual bool is_key_released(InputKey key) = 0; // Single release detection

    // Mouse state queries
    virtual MousePosition get_cursor_position() = 0;
    virtual bool is_mouse_button_down(MouseButton button) = 0;
    virtual bool is_mouse_button_up(MouseButton button) = 0;
    virtual bool is_mouse_button_pressed(MouseButton button) = 0;
    virtual bool is_mouse_button_released(MouseButton button) = 0;

    // Mouse actions
    virtual bool set_cursor_position(int x, int y) = 0;
    virtual bool move_mouse(int delta_x, int delta_y) = 0;
    virtual bool click_mouse_button(MouseButton button) = 0;
    virtual bool press_mouse_button(MouseButton button) = 0;
    virtual bool release_mouse_button(MouseButton button) = 0;
    virtual bool scroll_mouse(int delta) = 0;

    // Keyboard actions
    virtual bool press_key(InputKey key) = 0;
    virtual bool release_key(InputKey key) = 0;
    virtual bool tap_key(InputKey key) = 0; // Press and release quickly
    virtual bool type_text(const std::string& text) = 0;

    // Utility functions
    virtual void update() = 0; // Update internal state for press/release detection
    virtual bool is_initialized() const = 0;

protected:
    bool m_initialized;
    
    // Previous state tracking for single press/release detection
    bool m_prev_key_states[256];
    bool m_prev_mouse_states[5];
    
    void update_key_states();
    void update_mouse_states();
};

