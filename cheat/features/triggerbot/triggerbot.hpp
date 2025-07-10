#pragma once
#include <logger/logger.hpp>
#include <engine/sdk/math/vector.hpp>
#include <features/base_feature.hpp>
#include <features/triggerbot/settings/triggerbot_settings.hpp>
#include <input/adapter/base_input.h>
#include <memory>
#include <chrono>
#include <random>
#include <unordered_map>

class EntityCache;
class WorldCache;
class SettingsManager;
class Renderer;
class Engine;
class InputManager;
class Player;

struct EntityTrackingData {
    int shots_fired = 0;
    std::chrono::high_resolution_clock::time_point last_shot_time;
    std::chrono::high_resolution_clock::time_point cooldown_start_time;
    bool in_cooldown = false;
};

class TriggerbotFeature : public BaseFeature {
public:
    TriggerbotFeature(EntityCache* entity_cache, WorldCache* world_cache, Renderer* renderer, Engine* engine);
    ~TriggerbotFeature();
    
    bool initialize() override;
    void shutdown() override;
    void update() override;
    void render() override;
    void process_input() override;
    
    TriggerbotSettings& get_settings() { return m_settings; }
    const TriggerbotSettings& get_settings() const { return m_settings; }
    
    bool is_feature_enabled() const override { return m_settings.enabled; }

    void Shoot();
    
private:
    bool CanShootAtEntity(uintptr_t entity_id);
    bool IsReactionTimeReady();
    void UpdateEntityTracking(uintptr_t entity_id);
    int GetRandomReactionTime();
    
    TriggerbotSettings m_settings;
    InputManager* m_input_manager;
    
    // Entity tracking for burst control
    std::unordered_map<uintptr_t, EntityTrackingData> m_entity_tracking;
    
    // Reaction time system
    std::chrono::high_resolution_clock::time_point m_reaction_start_time;
    std::chrono::high_resolution_clock::time_point m_last_crosshair_change;
    uintptr_t m_last_crosshair_entity = 0;
    bool m_reaction_timer_active = false;
    int m_current_reaction_delay = 0;
    
    // Random number generation
    std::random_device m_random_device;
    std::mt19937 m_random_generator;
}; 