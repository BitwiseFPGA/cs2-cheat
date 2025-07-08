#pragma once
#include <logger/logger.hpp>
#include <access/adapter/base_access.hpp>

#include <string>
#include <memory>
#include <cstdint>
#include <vector>
#include <windows.h>

class AccessManager {
public:
    AccessManager();
    ~AccessManager();
    
    bool initialize();
    void shutdown();
    
    BaseMemoryAdapter* get_adapter() const { return m_adapter.get(); }
    
    template<typename T>
    bool read(uint64_t address, T& value) {
        return m_adapter ? m_adapter->read(address, value) : false;
    }
    
    template<typename T>
    T read(uint64_t address) {
        return m_adapter ? m_adapter->read<T>(address) : T{};
    }
    
    template<typename T>
    bool write(uint64_t address, const T& value) {
        return m_adapter ? m_adapter->write(address, value) : false;
    }
    
    bool read_memory(uint64_t address, void* buffer, size_t size) {
        return m_adapter ? m_adapter->read_memory(address, buffer, size) : false;
    }
    
    bool write_memory(uint64_t address, const void* buffer, size_t size) {
        return m_adapter ? m_adapter->write_memory(address, buffer, size) : false;
    }
    
    uint64_t get_module_base(const std::string& module_name) {
        return m_adapter ? m_adapter->get_module_base(module_name) : 0;
    }
    
    bool is_attached() const {
        return m_adapter ? m_adapter->is_attached() : false;
    }

    uint64_t get_process_base_address() {
        return m_adapter ? m_adapter->get_process_base_address() : 0;
    }
    
    ScatterHandle create_scatter_handle() {
        return m_adapter ? m_adapter->create_scatter_handle() : nullptr;
    }
    
    void close_scatter_handle(ScatterHandle handle) {
        if (m_adapter) {
            m_adapter->close_scatter_handle(handle);
        }
    }
    
    bool add_scatter_read(ScatterHandle handle, uint64_t address, void* buffer, size_t size) {
        return m_adapter ? m_adapter->add_scatter_read(handle, address, buffer, size) : false;
    }
    
    bool add_scatter_write(ScatterHandle handle, uint64_t address, const void* buffer, size_t size) {
        return m_adapter ? m_adapter->add_scatter_write(handle, address, buffer, size) : false;
    }
    
    bool scatter_read(ScatterHandle handle) {
        return m_adapter ? m_adapter->scatter_read(handle) : false;
    }
    
    bool scatter_write(ScatterHandle handle) {
        return m_adapter ? m_adapter->scatter_write(handle) : false;
    }

    uintptr_t find_pattern(uintptr_t module_base, const std::string& pattern, const size_t max_size = 0xFFFFFFFF);

    template<typename T>
    T find_pattern_and_extract(uintptr_t module_base, const std::string& pattern, size_t offset_position, size_t offset_size = sizeof(T), const size_t max_size = 0xFFFFFFFF) {
        uintptr_t pattern_address = find_pattern(module_base, pattern, max_size);
        if (pattern_address == 0) {
            logger::error("Pattern not found for offset extraction");
            return T{};
        }
        
        T value = extract_offset<T>(pattern_address, offset_position, offset_size);
        return value;
    }

    uintptr_t find_pattern_and_extract_relative(uintptr_t module_base, const std::string& pattern, 
                                               size_t offset_position, size_t instruction_length, 
                                               const size_t max_size = 0xFFFFFFFF) {
        uintptr_t pattern_address = find_pattern(module_base, pattern, max_size);
        if (pattern_address == 0) {
            logger::error("Pattern not found for relative address extraction");
            return 0;
        }
        
        return extract_relative_address(pattern_address, offset_position, instruction_length);
    }

    template<typename T>
    T extract_offset(uintptr_t address, size_t offset_position, size_t offset_size = sizeof(T)) {
        T value{};
        if (m_adapter && read_memory(address + offset_position, &value, offset_size)) {
            return value;
        }
        return T{};
    }

    uintptr_t extract_relative_address(uintptr_t instruction_address, size_t offset_position, size_t instruction_length);
   
private:
    std::unique_ptr<BaseMemoryAdapter> m_adapter;
};
