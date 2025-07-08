#include <access/adapter/local/winapi_access.hpp>
#include <logger/logger.hpp>

#include <iostream>
#include <algorithm>
#include <cctype>
#include <cwctype>
#include <sstream>

std::string sanitize_module_name(const std::string& name) {
    std::string sanitized = name;
    
    sanitized.erase(std::find(sanitized.begin(), sanitized.end(), '\0'), sanitized.end());
    
    while (!sanitized.empty() && (std::iscntrl(sanitized.back()) || std::isspace(sanitized.back()))) {
        sanitized.pop_back();
    }
    
    while (!sanitized.empty() && std::isspace(sanitized.front())) {
        sanitized.erase(0, 1);
    }
    
    std::transform(sanitized.begin(), sanitized.end(), sanitized.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    
    return sanitized;
}

std::wstring sanitize_module_name(const std::wstring& name) {
    std::wstring sanitized = name;
    
    sanitized.erase(std::find(sanitized.begin(), sanitized.end(), L'\0'), sanitized.end());
    
    while (!sanitized.empty() && (std::iswcntrl(sanitized.back()) || std::iswspace(sanitized.back()))) {
        sanitized.pop_back();
    }
    
    while (!sanitized.empty() && std::iswspace(sanitized.front())) {
        sanitized.erase(0, 1);
    }
    
    std::transform(sanitized.begin(), sanitized.end(), sanitized.begin(), 
                   [](wchar_t c) { return std::towlower(c); });
    
    return sanitized;
}

WinApiAccess::WinApiAccess() 
    : m_next_scatter_handle_id(1)
    , m_modules_cached(false)
{
}

WinApiAccess::~WinApiAccess() {
    shutdown();
}

bool WinApiAccess::initialize() {
    logger::info("Initializing local memory access");
    return true;
}

void WinApiAccess::shutdown() {
    detach_from_process();
    
    std::lock_guard<std::mutex> lock(m_scatter_mutex);
    m_scatter_handles.clear();
}

bool WinApiAccess::attach_to_process(const std::string& process_name) {
    uint32_t pid = find_process_id(process_name);
    if (pid == 0) {
        logger::error("Process not found: " + process_name);
        return false;
    }
    return attach_to_process(pid);
}

bool WinApiAccess::attach_to_process(const std::wstring& process_name) {
    uint32_t pid = find_process_id(process_name);
    if (pid == 0) {
        logger::error("Process not found: " + wstring_to_string(process_name));
        return false;
    }
    return attach_to_process(pid);
}

bool WinApiAccess::attach_to_process(uint32_t process_id) {
    if (m_attached) {
        detach_from_process();
    }
    
    m_process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
    if (!m_process_handle) {
        logger::error("Failed to open process with ID: " + std::to_string(process_id) + " Error: " + std::to_string(GetLastError()));
        return false;
    }
    
    m_process_id = process_id;
    m_attached = true;
    m_modules_cached = false;
    
    std::lock_guard<std::mutex> lock(m_modules_mutex);
    m_modules.clear();
    m_wmodules.clear();
    
    logger::success("Successfully attached to process ID: " + std::to_string(process_id));
    return true;
}

void WinApiAccess::detach_from_process() {
    if (m_process_handle) {
        CloseHandle(m_process_handle);
        m_process_handle = nullptr;
    }
    
    m_attached = false;
    m_process_id = 0;
    m_modules_cached = false;
    
    std::lock_guard<std::mutex> lock(m_modules_mutex);
    m_modules.clear();
    m_wmodules.clear();
}

uint64_t WinApiAccess::get_module_base(const std::string& module_name) {
    if (!refresh_module_list()) {
        return 0;
    }
    
    ModuleInfo info = get_module_info(module_name);
    return info.base_address;
}

uint64_t WinApiAccess::get_module_base(const std::wstring& module_name) {
    if (!refresh_module_list()) {
        return 0;
    }
    
    ModuleInfo info = get_module_info(module_name);
    return info.base_address;
}

size_t WinApiAccess::get_module_size(const std::string& module_name) {
    if (!refresh_module_list()) {
        return 0;
    }
    
    ModuleInfo info = get_module_info(module_name);
    return info.size;
}

size_t WinApiAccess::get_module_size(const std::wstring& module_name) {
    if (!refresh_module_list()) {
        return 0;
    }
    
    ModuleInfo info = get_module_info(module_name);
    return info.size;
}

bool WinApiAccess::read_memory(uint64_t address, void* buffer, size_t size) {
    if (!m_attached) {
        logger::debug("WinAPI: Cannot read memory - not attached to process");
        return false;
    }
    
    SIZE_T bytes_read = 0;
    BOOL result = ReadProcessMemory(m_process_handle, reinterpret_cast<LPCVOID>(address), buffer, size, &bytes_read);
    
    if (!result || bytes_read != size) {
        return false;
    }
    
    return true;
}

bool WinApiAccess::write_memory(uint64_t address, const void* buffer, size_t size) {
    if (!m_attached) {
        return false;
    }
    
    SIZE_T bytes_written = 0;
    BOOL result = WriteProcessMemory(m_process_handle, reinterpret_cast<LPVOID>(address), buffer, size, &bytes_written);
    return result && bytes_written == size;
}

bool WinApiAccess::read_string(uint64_t address, std::string& str, size_t max_length) {
    if (!m_attached) {
        return false;
    }
    
    std::vector<char> buffer(max_length + 1, 0);
    if (!read_memory(address, buffer.data(), max_length)) {
        return false;
    }
    
    str = std::string(buffer.data());
    return true;
}

bool WinApiAccess::read_wstring(uint64_t address, std::wstring& str, size_t max_length) {
    if (!m_attached) {
        return false;
    }
    
    std::vector<wchar_t> buffer(max_length + 1, 0);
    if (!read_memory(address, buffer.data(), max_length * sizeof(wchar_t))) {
        return false;
    }
    
    str = std::wstring(buffer.data());
    return true;
}

bool WinApiAccess::write_string(uint64_t address, const std::string& str) {
    if (!m_attached) {
        return false;
    }
    
    return write_memory(address, str.c_str(), str.length() + 1);
}

bool WinApiAccess::write_wstring(uint64_t address, const std::wstring& str) {
    if (!m_attached) {
        return false;
    }
    
    return write_memory(address, str.c_str(), (str.length() + 1) * sizeof(wchar_t));
}

ScatterHandle WinApiAccess::create_scatter_handle() {
    std::lock_guard<std::mutex> lock(m_scatter_mutex);
    
    uint32_t handle_id = m_next_scatter_handle_id++;
    m_scatter_handles[handle_id] = { handle_id, true };
    
    return reinterpret_cast<ScatterHandle>(static_cast<uintptr_t>(handle_id));
}

void WinApiAccess::close_scatter_handle(ScatterHandle handle) {
    std::lock_guard<std::mutex> lock(m_scatter_mutex);
    
    uint32_t handle_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle));
    auto it = m_scatter_handles.find(handle_id);
    if (it != m_scatter_handles.end()) {
        m_scatter_handles.erase(it);
    }
}

bool WinApiAccess::add_scatter_read(ScatterHandle handle, uint64_t address, void* buffer, size_t size) {
    if (!m_attached || !handle || !buffer || size == 0) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_scatter_mutex);
    
    uint32_t handle_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle));
    auto it = m_scatter_handles.find(handle_id);
    if (it == m_scatter_handles.end() || !it->second.in_use) {
        return false;
    }
    
    ScatterOperation op;
    op.address = address;
    op.buffer = buffer;
    op.size = size;
    op.is_write = false;
    op.success = false;
    
    it->second.operations.push_back(op);
    return true;
}

bool WinApiAccess::add_scatter_write(ScatterHandle handle, uint64_t address, const void* buffer, size_t size) {
    if (!m_attached || !handle || !buffer || size == 0) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_scatter_mutex);
    
    uint32_t handle_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle));
    auto it = m_scatter_handles.find(handle_id);
    if (it == m_scatter_handles.end() || !it->second.in_use) {
        return false;
    }
    
    ScatterOperation op;
    op.address = address;
    op.buffer = const_cast<void*>(buffer);
    op.size = size;
    op.is_write = true;
    op.success = false;
    
    it->second.operations.push_back(op);
    return true;
}

bool WinApiAccess::scatter_read(ScatterHandle handle) {
    if (!m_attached || !handle) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_scatter_mutex);
    
    uint32_t handle_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle));
    auto it = m_scatter_handles.find(handle_id);
    if (it == m_scatter_handles.end() || !it->second.in_use) {
        return false;
    }
    
    bool all_success = true;
    for (auto& op : it->second.operations) {
        if (!op.is_write) {
            op.success = read_memory(op.address, op.buffer, op.size);
            if (!op.success) {
                all_success = false;
            }
        }
    }
    
    it->second.operations.clear();
    return all_success;
}

bool WinApiAccess::scatter_write(ScatterHandle handle) {
    if (!m_attached || !handle) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_scatter_mutex);
    
    uint32_t handle_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle));
    auto it = m_scatter_handles.find(handle_id);
    if (it == m_scatter_handles.end() || !it->second.in_use) {
        return false;
    }
    
    bool all_success = true;
    for (auto& op : it->second.operations) {
        if (op.is_write) {
            op.success = write_memory(op.address, op.buffer, op.size);
            if (!op.success) {
                all_success = false;
            }
        }
    }
    
    it->second.operations.clear();
    return all_success;
}

bool WinApiAccess::is_valid_address(uint64_t address) {
    if (!m_attached) {
        return false;
    }
    
    MEMORY_BASIC_INFORMATION mbi;
    SIZE_T result = VirtualQueryEx(m_process_handle, reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi));
    
    return result != 0 && mbi.State == MEM_COMMIT && 
           (mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE));
}

bool WinApiAccess::is_attached() const {
    return m_attached;
}

uint32_t WinApiAccess::get_process_id() const {
    return m_process_id;
}

HANDLE WinApiAccess::get_process_handle() const {
    return m_process_handle;
}

uint64_t WinApiAccess::get_process_base_address() {
    if (!m_attached) {
        return 0;
    }
    
    if (!refresh_module_list()) {
        return 0;
    }
    
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_process_id);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    MODULEENTRY32W entry;
    entry.dwSize = sizeof(MODULEENTRY32W);
    
    uint64_t base_address = 0;
    if (Module32FirstW(snapshot, &entry)) {
        base_address = reinterpret_cast<uint64_t>(entry.modBaseAddr);
    }
    
    CloseHandle(snapshot);
    return base_address;
}

uint32_t WinApiAccess::find_process_id(const std::string& process_name) {
    return find_process_id(string_to_wstring(process_name));
}

uint32_t WinApiAccess::find_process_id(const std::wstring& process_name) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32W);
    
    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, process_name.c_str()) == 0) {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    
    CloseHandle(snapshot);
    return 0;
}

bool WinApiAccess::refresh_module_list() {
    if (!m_attached) {
        return false;
    }
    
    if (m_modules_cached) {
        return true;
    }
    
    std::lock_guard<std::mutex> lock(m_modules_mutex);
    
    m_modules.clear();
    m_wmodules.clear();
    
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_process_id);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    MODULEENTRY32W entry;
    entry.dwSize = sizeof(MODULEENTRY32W);
    
    if (Module32FirstW(snapshot, &entry)) {
        do {
            std::wstring wmodule_name = entry.szModule;
            std::string module_name = wstring_to_string(wmodule_name);

            module_name = sanitize_module_name(module_name);
            wmodule_name = sanitize_module_name(wmodule_name);
            
            ModuleInfo info;
            info.base_address = reinterpret_cast<uint64_t>(entry.modBaseAddr);
            info.size = entry.modBaseSize;
            
            m_modules[module_name] = info;
            m_wmodules[wmodule_name] = info;
            
        } while (Module32NextW(snapshot, &entry));
    }
    
    CloseHandle(snapshot);
    m_modules_cached = true;
    return true;
}

WinApiAccess::ModuleInfo WinApiAccess::get_module_info(const std::string& module_name) {
    std::lock_guard<std::mutex> lock(m_modules_mutex);
    
    std::string sanitized_name = sanitize_module_name(module_name);
    
    auto it = m_modules.find(sanitized_name);
    if (it != m_modules.end()) {
        return it->second;
    }
    
    return { 0, 0 };
}

WinApiAccess::ModuleInfo WinApiAccess::get_module_info(const std::wstring& module_name) {
    std::lock_guard<std::mutex> lock(m_modules_mutex);
    
    std::wstring sanitized_name = sanitize_module_name(module_name);
    
    auto it = m_wmodules.find(sanitized_name);
    if (it != m_wmodules.end()) {
        return it->second;
    }
    
    return { 0, 0 };
} 