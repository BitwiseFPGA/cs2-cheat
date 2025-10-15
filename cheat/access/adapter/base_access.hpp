#pragma once
#include <string>
#include <cstdint>
#include <windows.h>

using ScatterHandle = void*;

struct ScatterEntry {
    uint64_t address;
    void* buffer;
    size_t size;
    bool success;
};

class BaseMemoryAdapter {
public:
    BaseMemoryAdapter();
    virtual ~BaseMemoryAdapter();

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    virtual bool attach_to_process(const std::string& process_name) = 0;
    virtual bool attach_to_process(const std::wstring& process_name) = 0;
    virtual bool attach_to_process(uint32_t process_id) = 0;
    virtual void detach_from_process() = 0;

    virtual uint64_t get_module_base(const std::string& module_name) = 0;
    virtual uint64_t get_module_base(const std::wstring& module_name) = 0;
    virtual size_t get_module_size(const std::string& module_name) = 0;
    virtual size_t get_module_size(const std::wstring& module_name) = 0;

    template<typename T>
    bool read(uint64_t address, T& value) {
        return read_memory(address, &value, sizeof(T));
    }

    template<typename T>
    T read(uint64_t address) {
        T value{};
        read_memory(address, &value, sizeof(T));
        return value;
    }

    virtual bool read_memory(uint64_t address, void* buffer, size_t size) = 0;

    virtual bool read_string(uint64_t address, std::string& str, size_t max_length = 256) = 0;
    virtual bool read_wstring(uint64_t address, std::wstring& str, size_t max_length = 256) = 0;

    virtual ScatterHandle create_scatter_handle() = 0;
    virtual void close_scatter_handle(ScatterHandle handle) = 0;
    virtual bool add_scatter_read(ScatterHandle handle, uint64_t address, void* buffer, size_t size) = 0;
    virtual bool scatter_read(ScatterHandle handle) = 0;

    virtual bool is_valid_address(uint64_t address) = 0;
    virtual bool is_attached() const = 0;
    virtual uint32_t get_process_id() const = 0;
    virtual HANDLE get_process_handle() const = 0;
    virtual uint64_t get_process_base_address() = 0;

    static std::wstring string_to_wstring(const std::string& str);
    static std::string wstring_to_string(const std::wstring& wstr);

protected:
    bool m_attached;
    uint32_t m_process_id;
    VMM_HANDLE m_process_handle;
    std::string m_process_name;
};
