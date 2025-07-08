#pragma once
#include <input/adapter/base_input.h>

#include <unordered_map>

class WinApiInput : public BaseInputAdapter {
public:
    WinApiInput();
    ~WinApiInput() override;

    // BaseInputAdapter implementation
    bool initialize() override;
    void shutdown() override;

    // Key state queries
    bool is_key_down(InputKey key) override;
    bool is_key_up(InputKey key) override;
    bool is_key_pressed(InputKey key) override;
    bool is_key_released(InputKey key) override;

    // Mouse state queries
    MousePosition get_cursor_position() override;
    bool is_mouse_button_down(MouseButton button) override;
    bool is_mouse_button_up(MouseButton button) override;
    bool is_mouse_button_pressed(MouseButton button) override;
    bool is_mouse_button_released(MouseButton button) override;

    // Mouse actions
    bool set_cursor_position(int x, int y) override;
    bool move_mouse(int delta_x, int delta_y) override;
    bool click_mouse_button(MouseButton button) override;
    bool press_mouse_button(MouseButton button) override;
    bool release_mouse_button(MouseButton button) override;
    bool scroll_mouse(int delta) override;

    // Keyboard actions
    bool press_key(InputKey key) override;
    bool release_key(InputKey key) override;
    bool tap_key(InputKey key) override;
    bool type_text(const std::string& text) override;

    // Utility functions
    void update() override;
    bool is_initialized() const override;

private:
    // Helper methods
    UINT get_mouse_button_down_flag(MouseButton button);
    UINT get_mouse_button_up_flag(MouseButton button);
    WORD get_scan_code(InputKey key);
    bool send_input_event(const INPUT& input_event);
    
    // State tracking
    std::unordered_map<uint16_t, bool> m_current_key_states;
    std::unordered_map<uint8_t, bool> m_current_mouse_states;
};

