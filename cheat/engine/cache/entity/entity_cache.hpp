#pragma once
#include <logger/logger.hpp>
#include <engine/sdk/types/entity_impl.hpp>
#include <access/adapter/base_adapter.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <chrono>

class AccessManager;
class AutoScatterSystem;

constexpr int MAX_ENTITIES = 1024;
constexpr int MAX_PLAYERS = 64;

class EntityCache {
public:
    EntityCache(AccessManager* access_manager);
    ~EntityCache();
    
    bool initialize();
    void shutdown();
    
    void update();
    void clear();
            
private:
    AccessManager* m_access_manager;
    bool m_initialized;
    
    std::chrono::milliseconds m_last_update;

    ScatterHandle m_scatter_handle = nullptr;

    std::uintptr_t m_client_dll_base;

    std::uintptr_t m_local_player_ptr;
    std::uintptr_t m_entity_list_ptr;
    std::uintptr_t m_global_vars_ptr;

    std::vector<uintptr_t> m_list_entries;
};

