#pragma once
#include <logger/logger.hpp>
#include <engine/sdk/math/vector.hpp>
#include <features/base_feature.hpp>
#include <features/aimbot/settings/aimbot_settings.hpp>
#include <input/adapter/base_input.h>

#include <memory>
#include <vector>

class EntityCache;
class WorldCache;
class SettingsManager;
class Renderer;
class Engine;
class InputManager;
class Player;

class AimbotFeature : public BaseFeature {
public:
    AimbotFeature(EntityCache* entity_cache, WorldCache* world_cache, Renderer* renderer, Engine* engine);
    ~AimbotFeature();
    
    bool initialize() override;
    void shutdown() override;
    void update() override;
    void render() override;
    void process_input() override;
    
    AimbotSettings& get_settings() { return m_settings; }
    const AimbotSettings& get_settings() const { return m_settings; }
    
    bool is_feature_enabled() const override { return m_settings.enabled; }
    
private:
    AimbotSettings m_settings;
    InputManager* m_input_manager;
    
    // Target tracking
    Vector2 m_target_position;
    bool m_has_target;
    float m_current_target_distance;
    
    // Movement accumulation for precise smoothing
    float m_movement_remainder_x;
    float m_movement_remainder_y;
    Vector3 m_old_punch_angle; // Store previous punch angle for recoil control
    
    // Helper methods
    bool should_aim() const;
    void aim_at_screen_position(const Vector2& target_pos);
    
    // Bone targeting methods
    Vector3 get_bone_position(const Player& player, AimbotBone bone) const;
    std::vector<Vector3> get_target_bones(const Player& player) const;
    bool is_target_visible(const Vector3& local_eye_pos, const Vector3& target_pos) const;
};
