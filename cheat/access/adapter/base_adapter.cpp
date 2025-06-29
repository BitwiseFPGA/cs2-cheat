#include <access/adapter/base_adapter.hpp>

BaseMemoryAdapter::BaseMemoryAdapter()
    : m_attached(false)
    , m_process_id(0)
    , m_process_handle(nullptr)
{
}

BaseMemoryAdapter::~BaseMemoryAdapter()
{
}

std::wstring BaseMemoryAdapter::string_to_wstring(const std::string& str) {
    if (str.empty()) return std::wstring();

    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
    return wstr;
}

std::string BaseMemoryAdapter::wstring_to_string(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();

    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string str(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size, nullptr, nullptr);
    return str;
}
