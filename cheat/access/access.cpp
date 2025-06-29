#include <access/access.hpp>
#include <config/build/settings.hpp>
#include <logger/logger.hpp>
#include <access/adapter/local/winapi.hpp>
#include <access/adapter/remote/dma.hpp>

AccessManager::AccessManager()
    : m_adapter(nullptr)
{
}

AccessManager::~AccessManager() {
    if (m_adapter) {
        shutdown();
    }
}

bool AccessManager::initialize() {
    logger::log_step("Memory Manager Init", "Setting up memory access");
    
    try {
#if USE_DMA_MEMORY
        logger::info("Using DMA memory access");
        m_adapter = std::make_unique<DmaMemoryAccess>();
#else
        logger::info("Using local memory access");
        m_adapter = std::make_unique<WinApiAccess>();
#endif
        
        if (!m_adapter->initialize()) {
            logger::log_failure("Memory Manager", "Failed to initialize memory access");
            return false;
        }
        
        std::string target_process = TARGET_PROCESS_NAME;
        if (!target_process.empty()) {
            if (!m_adapter->attach_to_process(target_process)) {
                logger::log_failure("Memory Manager", "Failed to attach to process: " + target_process);
                return false;
            }
            logger::success("Attached to process: " + target_process);
        }
        
        logger::success("Memory Manager initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        logger::log_failure("Memory Manager", e.what());
        return false;
    }
}

void AccessManager::shutdown() {
    logger::log_step("Memory Manager Shutdown", "Cleaning up memory access");
    
    if (m_adapter) {
        m_adapter->shutdown();
        m_adapter.reset();
    }
    
    logger::success("Memory Manager shutdown completed");
}
