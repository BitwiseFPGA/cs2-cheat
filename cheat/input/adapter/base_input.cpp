#include "base_input.h"
#include <cstring>

BaseInputAdapter::BaseInputAdapter()
    : m_initialized(false)
{
    // Initialize state arrays
    std::memset(m_prev_key_states, 0, sizeof(m_prev_key_states));
    std::memset(m_prev_mouse_states, 0, sizeof(m_prev_mouse_states));
}

BaseInputAdapter::~BaseInputAdapter() {
    
}

void BaseInputAdapter::update_key_states() {
    // This should be called by derived classes to update previous key states
    // for proper press/release detection
    for (int i = 0; i < 256; ++i) {
        m_prev_key_states[i] = is_key_down(static_cast<InputKey>(i));
    }
}

void BaseInputAdapter::update_mouse_states() {
    // Update previous mouse button states
    for (int i = 0; i < 5; ++i) {
        m_prev_mouse_states[i] = is_mouse_button_down(static_cast<MouseButton>(i));
    }
}
