#pragma once
#include <access/access.hpp>

#include <string>
#include <unordered_map>

class OffsetExtractor {
public:
    OffsetExtractor(AccessManager* access_manager);

    bool extract_offsets();

    uint32_t get_offset(const std::string& name);

private:
    AccessManager* m_access_manager;
    std::unordered_map<std::string, uint32_t> m_offsets;
};