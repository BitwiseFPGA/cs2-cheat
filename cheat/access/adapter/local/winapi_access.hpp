#pragma once
#include <access/access.hpp>

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <map>
#include <mutex>
#include <vector>

class WinApiAccess : public BaseMemoryAdapter {
public:
    WinApiAccess();
    virtual ~WinApiAccess();
    
    bool initialize() override;
    void shutdown() override;
    
    bool attach_to_process(const std::string& process_name) override;
    bool attach_to_process(const std::wstring& process_name) override;
    bool attach_to_process(uint32_t process_id) override;
    void detach_from_process() override;
    
    uint64_t get_module_base(const std::string& module_name) override;
    uint64_t get_module_base(const std::wstring& module_name) override;
    size_t get_module_size(const std::string& module_name) override;
    size_t get_module_size(const std::wstring& module_name) override;
    
    bool read_memory(uint64_t address, void* buffer, size_t size) override;
    
    bool read_string(uint64_t address, std::string& str, size_t max_length = 256) override;
    bool read_wstring(uint64_t address, std::wstring& str, size_t max_length = 256) override;
    
    ScatterHandle create_scatter_handle() override;
    void close_scatter_handle(ScatterHandle handle) override;
    bool add_scatter_read(ScatterHandle handle, uint64_t address, void* buffer, size_t size) override;
    bool scatter_read(ScatterHandle handle) override;
    
    bool is_valid_address(uint64_t address) override;
    bool is_attached() const override;
    uint32_t get_process_id() const override;
    HANDLE get_process_handle() const override;
    uint64_t get_process_base_address() override;
    
private:
    struct ModuleInfo {
        uint64_t base_address;
        size_t size;
    };
    
    struct ScatterOperation {
        uint64_t address;
        void* buffer;
        size_t size;
        bool is_write;
        bool success;
    };
    
    struct ScatterHandleData {
        uint32_t id;
        bool in_use;
        std::vector<ScatterOperation> operations;
    };
    
    uint32_t find_process_id(const std::string& process_name);
    uint32_t find_process_id(const std::wstring& process_name);
    bool refresh_module_list();
    ModuleInfo get_module_info(const std::string& module_name);
    ModuleInfo get_module_info(const std::wstring& module_name);
    
    std::map<std::string, ModuleInfo> m_modules;
    std::map<std::wstring, ModuleInfo> m_wmodules;
    std::map<uint32_t, ScatterHandleData> m_scatter_handles;
    std::mutex m_modules_mutex;
    std::mutex m_scatter_mutex;
    uint32_t m_next_scatter_handle_id;
    bool m_modules_cached;
};
