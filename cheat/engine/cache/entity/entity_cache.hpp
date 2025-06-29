#pragma once
#include <logger/logger.hpp>
#include <engine/sdk/types/impl/entity_impl.hpp>
#include <engine/sdk/types/impl/player_impl.hpp>
#include <vector>
#include <memory>
#include <unordered_map>
#include <chrono>

class AccessManager;
class AutoScatterSystem;

class EntityCache {
public:
    EntityCache(AccessManager* access_manager);
    ~EntityCache();
    
    bool initialize();
    void shutdown();
    
    void update();
    void clear();
    
    const std::vector<std::shared_ptr<GameEntity>>& get_entities() const { return m_entities; }
    const std::vector<std::shared_ptr<GamePlayer>>& get_players() const { return m_players; }
    
    std::shared_ptr<GamePlayer> get_local_player() const { return m_local_player; }
    
private:
    AccessManager* m_access_manager;
    bool m_initialized;
    
    std::chrono::milliseconds m_last_update;

// Shared internal data: 
    std::vector<std::shared_ptr<GameEntity>> m_entities;
    std::vector<std::shared_ptr<GamePlayer>> m_players;
    std::shared_ptr<GamePlayer> m_local_player;
};

