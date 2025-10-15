#include <access/access.hpp>
#include <config/build/settings.hpp>
#include <logger/logger.hpp>
#include <access/adapter/local/winapi_access.hpp>
#include <access/adapter/remote/dma.hpp>

#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdio>

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
        logger::info("Using remote memory access");
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

uintptr_t AccessManager::find_pattern(uintptr_t module_base, const std::string& pattern, const size_t max_size) {
    if (!m_adapter || !m_adapter->is_attached()) {
        logger::error("Memory adapter not available for pattern finding");
        return 0;
    }

    if (module_base == 0) {
        logger::error("Invalid module base address for pattern finding");
        return 0;
    }

    std::vector<uint8_t> pattern_bytes;
    std::vector<bool> pattern_mask;
    
    std::istringstream pattern_stream(pattern);
    std::string byte_str;
    
    while (pattern_stream >> byte_str) {
        if (byte_str == "?" || byte_str == "??") {
            pattern_bytes.push_back(0x00);
            pattern_mask.push_back(false);
        } else {
            try {
                uint8_t byte_val = static_cast<uint8_t>(std::stoul(byte_str, nullptr, 16));
                pattern_bytes.push_back(byte_val);
                pattern_mask.push_back(true);
            } catch (const std::exception& e) {
                logger::error("Invalid pattern format: " + std::string(e.what()));
                return 0;
            }
        }
    }

    if (pattern_bytes.empty()) {
        logger::error("Empty pattern provided");
        return 0;
    }

    const size_t pattern_size = pattern_bytes.size();
    const size_t chunk_size = 0x1000;
    const size_t overlap_size = pattern_size - 1;
    
    std::vector<uint8_t> buffer(chunk_size);
    uintptr_t current_address = module_base;
    uintptr_t end_address = module_base + max_size;

    logger::debug("Searching for pattern starting at: 0x" + 
                 std::to_string(module_base) + " with max size: " + std::to_string(max_size));

    while (current_address < end_address) {
        size_t bytes_to_read = std::min<size_t>(chunk_size, static_cast<size_t>(end_address - current_address));
        
        if (!read_memory(current_address, buffer.data(), bytes_to_read)) {
            current_address += chunk_size;
            continue;
        }

        for (size_t i = 0; i <= bytes_to_read - pattern_size; ++i) {
            bool pattern_found = true;
            
            for (size_t j = 0; j < pattern_size; ++j) {
                if (pattern_mask[j] && buffer[i + j] != pattern_bytes[j]) {
                    pattern_found = false;
                    break;
                }
            }
            
            if (pattern_found) {
                uintptr_t found_address = current_address + i;
                logger::success("Pattern found at: 0x" + std::to_string(found_address));
                return found_address;
            }
        }

        if (current_address + chunk_size - overlap_size >= end_address) {
            break;
        }
        current_address += chunk_size - overlap_size;
    }

    logger::warning("Pattern not found in specified range");
    return 0;
}

uintptr_t AccessManager::extract_relative_address(uintptr_t instruction_address, size_t offset_position, size_t instruction_length) {
    int32_t relative_offset = extract_offset<int32_t>(instruction_address, offset_position);
    if (relative_offset == 0) {
        logger::error("Failed to extract relative offset");
        return 0;
    }
    
    uintptr_t target_address = instruction_address + instruction_length + relative_offset;
    logger::debug("Extracted relative address: 0x" + std::to_string(target_address) + 
                 " (base: 0x" + std::to_string(instruction_address) + 
                 ", offset: 0x" + std::to_string(relative_offset) + ")");
    return target_address;
}
