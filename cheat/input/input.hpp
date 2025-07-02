#pragma once
#include <logger/logger.hpp>
#include <input/adapter/base_input.h>
#include <string>
#include <memory>

class InputManager {
public:
    InputManager();
    ~InputManager();
    
    bool initialize();
    void shutdown();
    void update(); // Call this regularly to update input state
    
    BaseInputAdapter* get_adapter() const { return m_adapter.get(); }
    
    // Key state queries
    bool is_key_down(InputKey key) {
        return m_adapter ? m_adapter->is_key_down(key) : false;
    }
    
    bool is_key_up(InputKey key) {
        return m_adapter ? m_adapter->is_key_up(key) : false;
    }
    
    bool is_key_pressed(InputKey key) {
        return m_adapter ? m_adapter->is_key_pressed(key) : false;
    }
    
    bool is_key_released(InputKey key) {
        return m_adapter ? m_adapter->is_key_released(key) : false;
    }
    
    // Mouse state queries
    MousePosition get_cursor_position() {
        return m_adapter ? m_adapter->get_cursor_position() : MousePosition{0, 0};
    }
    
    bool is_mouse_button_down(MouseButton button) {
        return m_adapter ? m_adapter->is_mouse_button_down(button) : false;
    }
    
    bool is_mouse_button_up(MouseButton button) {
        return m_adapter ? m_adapter->is_mouse_button_up(button) : false;
    }
    
    bool is_mouse_button_pressed(MouseButton button) {
        return m_adapter ? m_adapter->is_mouse_button_pressed(button) : false;
    }
    
    bool is_mouse_button_released(MouseButton button) {
        return m_adapter ? m_adapter->is_mouse_button_released(button) : false;
    }
    
    // Mouse actions
    bool set_cursor_position(int x, int y) {
        return m_adapter ? m_adapter->set_cursor_position(x, y) : false;
    }
    
    bool move_mouse(int delta_x, int delta_y) {
        return m_adapter ? m_adapter->move_mouse(delta_x, delta_y) : false;
    }
    
    bool click_mouse_button(MouseButton button) {
        return m_adapter ? m_adapter->click_mouse_button(button) : false;
    }
    
    bool press_mouse_button(MouseButton button) {
        return m_adapter ? m_adapter->press_mouse_button(button) : false;
    }
    
    bool release_mouse_button(MouseButton button) {
        return m_adapter ? m_adapter->release_mouse_button(button) : false;
    }
    
    bool scroll_mouse(int delta) {
        return m_adapter ? m_adapter->scroll_mouse(delta) : false;
    }
    
    // Keyboard actions
    bool press_key(InputKey key) {
        return m_adapter ? m_adapter->press_key(key) : false;
    }
    
    bool release_key(InputKey key) {
        return m_adapter ? m_adapter->release_key(key) : false;
    }
    
    bool tap_key(InputKey key) {
        return m_adapter ? m_adapter->tap_key(key) : false;
    }
    
    bool type_text(const std::string& text) {
        return m_adapter ? m_adapter->type_text(text) : false;
    }
    
    bool is_initialized() const {
        return m_adapter ? m_adapter->is_initialized() : false;
    }

private:
    std::unique_ptr<BaseInputAdapter> m_adapter;
};

