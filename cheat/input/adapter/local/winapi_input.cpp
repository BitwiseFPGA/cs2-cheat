#include "winapi_input.h"
#include <thread>
#include <chrono>
#include <logger/logger.hpp>

WinApiInput::WinApiInput() : BaseInputAdapter() {
}

WinApiInput::~WinApiInput() {
}

bool WinApiInput::initialize() {
    if (m_initialized) {
        return true;
    }
    
    m_initialized = true;
    return true;
}

void WinApiInput::shutdown() {
    if (!m_initialized) {
        return;
    }
    
    m_current_key_states.clear();
    m_current_mouse_states.clear();
    m_initialized = false;
}

bool WinApiInput::is_key_down(InputKey key) {
    if (!m_initialized) {
        return false;
    }
    
    SHORT state = GetAsyncKeyState(static_cast<int>(key));
    return (state & 0x8000) != 0;
}

bool WinApiInput::is_key_up(InputKey key) {
    return !is_key_down(key);
}

bool WinApiInput::is_key_pressed(InputKey key) {
    if (!m_initialized) {
        return false;
    }
    
    uint16_t key_code = static_cast<uint16_t>(key);
    bool current_state = is_key_down(key);
    bool previous_state = m_current_key_states[key_code];
    
    return current_state && !previous_state;
}

bool WinApiInput::is_key_released(InputKey key) {
    if (!m_initialized) {
        return false;
    }
    
    uint16_t key_code = static_cast<uint16_t>(key);
    bool current_state = is_key_down(key);
    bool previous_state = m_current_key_states[key_code];
    
    return !current_state && previous_state;
}

MousePosition WinApiInput::get_cursor_position() {
    POINT point;
    if (GetCursorPos(&point)) {
        return { point.x, point.y };
    }
    return { 0, 0 };
}

bool WinApiInput::is_mouse_button_down(MouseButton button) {
    if (!m_initialized) {
        return false;
    }
    
    InputKey key_code;
    switch (button) {
        case MouseButton::Left: key_code = InputKey::MouseLeft; break;
        case MouseButton::Right: key_code = InputKey::MouseRight; break;
        case MouseButton::Middle: key_code = InputKey::MouseMiddle; break;
        case MouseButton::X1: key_code = InputKey::MouseX1; break;
        case MouseButton::X2: key_code = InputKey::MouseX2; break;
        default: return false;
    }
    
    return is_key_down(key_code);
}

bool WinApiInput::is_mouse_button_up(MouseButton button) {
    return !is_mouse_button_down(button);
}

bool WinApiInput::is_mouse_button_pressed(MouseButton button) {
    if (!m_initialized) {
        return false;
    }
    
    uint8_t button_code = static_cast<uint8_t>(button);
    bool current_state = is_mouse_button_down(button);
    bool previous_state = m_current_mouse_states[button_code];
    
    return current_state && !previous_state;
}

bool WinApiInput::is_mouse_button_released(MouseButton button) {
    if (!m_initialized) {
        return false;
    }
    
    uint8_t button_code = static_cast<uint8_t>(button);
    bool current_state = is_mouse_button_down(button);
    bool previous_state = m_current_mouse_states[button_code];
    
    return !current_state && previous_state;
}

bool WinApiInput::set_cursor_position(int x, int y) {
    if (!m_initialized) {
        return false;
    }
    
    return SetCursorPos(x, y) != 0;
}

bool WinApiInput::move_mouse(int delta_x, int delta_y) {
    if (!m_initialized) {
        return false;
    }
    
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dx = delta_x;
    input.mi.dy = delta_y;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    
    return send_input_event(input);
}

bool WinApiInput::click_mouse_button(MouseButton button) {
    return press_mouse_button(button) && release_mouse_button(button);
}

bool WinApiInput::press_mouse_button(MouseButton button) {
    if (!m_initialized) {
        return false;
    }
    
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = get_mouse_button_down_flag(button);
    
    return send_input_event(input);
}

bool WinApiInput::release_mouse_button(MouseButton button) {
    if (!m_initialized) {
        return false;
    }
    
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = get_mouse_button_up_flag(button);
    
    return send_input_event(input);
}

bool WinApiInput::scroll_mouse(int delta) {
    if (!m_initialized) {
        return false;
    }
    
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
    input.mi.mouseData = delta * WHEEL_DELTA;
    
    return send_input_event(input);
}

bool WinApiInput::press_key(InputKey key) {
    if (!m_initialized) {
        return false;
    }
    
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = static_cast<WORD>(key);
    input.ki.wScan = get_scan_code(key);
    input.ki.dwFlags = 0;
    
    return send_input_event(input);
}

bool WinApiInput::release_key(InputKey key) {
    if (!m_initialized) {
        return false;
    }
    
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = static_cast<WORD>(key);
    input.ki.wScan = get_scan_code(key);
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    
    return send_input_event(input);
}

bool WinApiInput::tap_key(InputKey key) {
    if (!press_key(key)) {
        return false;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    return release_key(key);
}

bool WinApiInput::type_text(const std::string& text) {
    if (!m_initialized) {
        return false;
    }
    
    for (char c : text) {
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = 0;
        input.ki.wScan = c;
        input.ki.dwFlags = KEYEVENTF_UNICODE;
        
        if (!send_input_event(input)) {
            return false;
        }
        
        // Release
        input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        if (!send_input_event(input)) {
            return false;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    return true;
}

void WinApiInput::update() {
    if (!m_initialized) {
        return;
    }
    
    // Update key states
    for (int i = 0; i < 256; ++i) {
        m_current_key_states[i] = is_key_down(static_cast<InputKey>(i));
    }
    
    // Update mouse states
    for (int i = 0; i < 5; ++i) {
        m_current_mouse_states[i] = is_mouse_button_down(static_cast<MouseButton>(i));
    }
}

bool WinApiInput::is_initialized() const {
    return m_initialized;
}

UINT WinApiInput::get_mouse_button_down_flag(MouseButton button) {
    switch (button) {
        case MouseButton::Left: return MOUSEEVENTF_LEFTDOWN;
        case MouseButton::Right: return MOUSEEVENTF_RIGHTDOWN;
        case MouseButton::Middle: return MOUSEEVENTF_MIDDLEDOWN;
        case MouseButton::X1: return MOUSEEVENTF_XDOWN;
        case MouseButton::X2: return MOUSEEVENTF_XDOWN;
        default: return 0;
    }
}

UINT WinApiInput::get_mouse_button_up_flag(MouseButton button) {
    switch (button) {
        case MouseButton::Left: return MOUSEEVENTF_LEFTUP;
        case MouseButton::Right: return MOUSEEVENTF_RIGHTUP;
        case MouseButton::Middle: return MOUSEEVENTF_MIDDLEUP;
        case MouseButton::X1: return MOUSEEVENTF_XUP;
        case MouseButton::X2: return MOUSEEVENTF_XUP;
        default: return 0;
    }
}

WORD WinApiInput::get_scan_code(InputKey key) {
    return MapVirtualKey(static_cast<UINT>(key), MAPVK_VK_TO_VSC);
}

bool WinApiInput::send_input_event(const INPUT& input_event) {
    return SendInput(1, const_cast<INPUT*>(&input_event), sizeof(INPUT)) == 1;
}
