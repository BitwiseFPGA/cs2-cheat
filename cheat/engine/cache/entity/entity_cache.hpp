#pragma once
#include <logger/logger.hpp>
#include <engine/engine.hpp>
#include <engine/sdk/types/entity.hpp>
#include <engine/sdk/types/player.hpp>
#include <engine/sdk/types/smoke.hpp>
#include <engine/sdk/math/matrix.hpp>
#include <access/adapter/base_access.hpp>
#include <engine/physics/traceline.hpp>

#include <vector>
#include <memory>
#include <unordered_map>
#include <chrono>

class Engine;
class AccessManager;
class AutoScatterSystem;
class TracelineManager;

constexpr int MAX_ENTITIES = 1024;
constexpr int MAX_PLAYERS = 64;

class EntityCache {
public:
    EntityCache(AccessManager* access_manager, Engine* engine);
    ~EntityCache();
    
    bool initialize();
    void shutdown();
    
    void update();
    void update_frame();
    void clear();
    
    std::vector<GameEntity>& get_entities() { return m_entities; }
    std::vector<Player>& get_players() { return m_players; }
    std::vector<SmokeGrenade>& get_smokes() { return m_smokes; }

    Player* get_local_player() { return m_local_player; }
    GameEntity* get_c4() { return m_c4; }

    uintptr_t get_client_dll_base() const { return m_client_dll_base; }
    uintptr_t get_local_player_ptr() const { return m_local_player_ptr; }
    uintptr_t get_entity_list_ptr() const { return m_entity_list_ptr; }
    uintptr_t get_global_vars_ptr() const { return m_global_vars_ptr; }
    uintptr_t get_crosshair_entity() const { return m_crosshair_entity; }
            
private:
    void fetch_globals();
    void fetch_entities();
    void fetch_entity_data(std::vector<GameEntity*>& entities_to_update);
    void fetch_player_data(std::vector<Player*>& players_to_update);
    void fetch_other_entity_data(const std::vector<GameEntity>& entities);
    
    Engine* m_engine;
    AccessManager* m_access_manager;
    bool m_initialized;
    
    std::chrono::milliseconds m_last_update;

    ScatterHandle m_scatter_handle = nullptr;

    uintptr_t m_client_dll_base = 0;

    uintptr_t m_local_player_ptr = 0;
    uintptr_t m_entity_list_ptr = 0;
    uintptr_t m_global_vars_ptr = 0;

    int32_t m_crosshair = 0;
    uintptr_t m_crosshair_entry = 0;
    uintptr_t m_crosshair_entity = 0;
    
    std::vector<uintptr_t> m_list_entries;
    std::vector<GameEntity> m_entities;
    std::vector<SmokeGrenade> m_smokes;
    std::vector<Player> m_players;
    
    Player* m_local_player = nullptr;
    GameEntity* m_c4 = nullptr;
    
    std::unordered_map<uint64_t, GameEntity> m_entity_cache_buffer;
    std::unordered_map<uint64_t, Player> m_player_cache_buffer;
    std::vector<GameEntity> m_new_entities_buffer;
    std::vector<Player> m_new_players_buffer;
    std::vector<GameEntity*> m_entities_to_update_buffer;
    std::vector<Player*> m_players_to_update_buffer;
};

