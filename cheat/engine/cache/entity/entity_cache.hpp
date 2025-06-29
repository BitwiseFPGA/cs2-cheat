#pragma once
#include <logger/logger.hpp>
#include <engine/sdk/types/entity_impl.hpp>
#include <engine/sdk/types/player_impl.hpp>
#include <engine/sdk/math/matrix.hpp>
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
    void update_frame();
    void clear();
    
    // Getters
    const std::vector<GameEntity>& get_entities() const { return m_entities; }
    const std::vector<Player>& get_players() const { return m_players; }
    Player* get_local_player() { return m_local_player; }
    GameEntity* get_c4() { return m_c4; }
    uintptr_t get_client_dll_base() const { return m_client_dll_base; }
    uintptr_t get_local_player_ptr() const { return m_local_player_ptr; }
    uintptr_t get_entity_list_ptr() const { return m_entity_list_ptr; }
    uintptr_t get_global_vars_ptr() const { return m_global_vars_ptr; }
            
private:
    void fetch_globals();
    void fetch_entities();
    void fetch_entity_data(std::vector<GameEntity*>& entities_to_update);
    void fetch_player_data(std::vector<Player*>& players_to_update);
    
    AccessManager* m_access_manager;
    bool m_initialized;
    
    std::chrono::milliseconds m_last_update;

    ScatterHandle m_scatter_handle = nullptr;

    uintptr_t m_client_dll_base = 0;

    // Global data
    uintptr_t m_local_player_ptr = 0;
    uintptr_t m_entity_list_ptr = 0;
    uintptr_t m_global_vars_ptr = 0;
    
    // Entity data
    std::vector<uintptr_t> m_list_entries;
    std::vector<GameEntity> m_entities;
    std::vector<Player> m_players;
    
    // Quick access pointers
    Player* m_local_player = nullptr;
    GameEntity* m_c4 = nullptr;
    
    // Caching buffers
    std::unordered_map<uint64_t, GameEntity> m_entity_cache_buffer;
    std::unordered_map<uint64_t, Player> m_player_cache_buffer;
    std::vector<GameEntity> m_new_entities_buffer;
    std::vector<Player> m_new_players_buffer;
    std::vector<GameEntity*> m_entities_to_update_buffer;
    std::vector<Player*> m_players_to_update_buffer;
};

