#pragma once
#include <logger/logger.hpp>
#include <engine/sdk/math/vector.hpp>
#include <features/base_feature.hpp>
#include <features/triggerbot/settings/triggerbot_settings.hpp>
#include <input/adapter/base_input.h>
#include <memory>
#include <chrono>
#include <random>

class EntityCache;
class WorldCache;
class SettingsManager;
class Renderer;
class Engine;
class InputManager;
class Player;

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
    
private:
    TriggerbotSettings m_settings;
    InputManager* m_input_manager;
    
    // Target tracking
    bool m_target_in_crosshair;
    Vector3 m_target_position;
    float m_target_distance;
    std::string m_target_info;
    
    // Trigger state
    bool m_should_trigger;
    bool m_trigger_queued;
    std::chrono::high_resolution_clock::time_point m_trigger_time;
    std::chrono::high_resolution_clock::time_point m_last_shot_time;
    
    // Random delay generation
    std::random_device m_rd;
    mutable std::mt19937 m_gen;
    mutable std::uniform_real_distribution<float> m_delay_dist;
    
    // Helper methods
    bool should_trigger() const;
    bool is_crosshair_on_target(const Vector3& target_pos, Vector2& screen_pos) const;
    bool is_target_valid(const Player& player, const Player& local_player) const;
    bool is_target_visible(const Vector3& local_eye_pos, const Vector3& target_pos) const;
    Vector3 get_target_bone_position(const Player& player) const;
    bool check_weapon_compatibility() const;
    bool check_safety_conditions(const Player& local_player) const;
    void queue_trigger_shot();
    void execute_trigger_shot();
    float generate_random_delay() const;
    
    // Constants
    static constexpr float MIN_SHOT_INTERVAL_MS = 100.0f; // Minimum time between shots
}; 