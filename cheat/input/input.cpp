#include <input/input.hpp>
#include <config/build/settings.hpp>
#include <logger/logger.hpp>
#include <input/adapter/local/winapi_input.h>
#include <input/adapter/remote/kmbox.hpp>

InputManager::InputManager()
    : m_adapter(nullptr)
{
}

InputManager::~InputManager() {
    if (m_adapter) {
        shutdown();
    }
}

bool InputManager::initialize() {
    logger::log_step("Input Manager Init", "Setting up input system");
    
    try {
#if USE_REMOTE_INPUT
        logger::info("Using remote input (KmBox)");
        m_adapter = std::make_unique<KmBoxInput>();
#else
        logger::info("Using local input (WinAPI)");
        m_adapter = std::make_unique<WinApiInput>();
#endif
        
        if (!m_adapter->initialize()) {
            logger::log_failure("Input Manager", "Failed to initialize input adapter");
            return false;
        }
        
        logger::success("Input Manager initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        logger::log_failure("Input Manager", e.what());
        return false;
    }
}

void InputManager::shutdown() {
    logger::log_step("Input Manager Shutdown", "Cleaning up input system");
    
    if (m_adapter) {
        m_adapter->shutdown();
        m_adapter.reset();
    }
    
    logger::success("Input Manager shutdown completed");
}

void InputManager::update() {
    if (m_adapter) {
        m_adapter->update();
    }
}
