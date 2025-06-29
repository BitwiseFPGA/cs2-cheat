#pragma once
#include <logger/logger.hpp>
#include <access/adapter/base_adapter.hpp>
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

private:
    std::unique_ptr<BaseMemoryAdapter> m_adapter;
};
