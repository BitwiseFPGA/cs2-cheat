#include <features/aimbot/aimbot.hpp>
#include <engine/cache/entity/entity_cache.hpp>
#include <engine/cache/world/world_cache.hpp>
#include <renderer/renderer.hpp>
#include <input/input.hpp>
#include <engine/engine.hpp>
#include <engine/sdk/types/player_impl.hpp>
#include <engine/physics/traceline.hpp>
#include <logger/logger.hpp>
#include <algorithm>
#include <cmath>
#include <cfloat>

AimbotFeature::AimbotFeature(EntityCache* entity_cache, WorldCache* world_cache, Renderer* renderer, Engine* engine)
    : BaseFeature("Aimbot", entity_cache, world_cache, renderer, engine)
    , m_input_manager(nullptr)
    , m_target_position(0, 0)
    , m_has_target(false)
    , m_current_target_distance(0.0f)
{
    logger::debug("Aimbot Feature created");
}

AimbotFeature::~AimbotFeature() {
    logger::debug("Aimbot Feature destroyed");
}

bool AimbotFeature::initialize() {
    logger::debug("Initializing Aimbot Feature");
    
    if (m_engine) {
        m_input_manager = m_engine->get_input_manager();
    }
    
    if (!m_input_manager) {
        logger::error("Failed to get InputManager for Aimbot");
        return false;
    }
    
    m_initialized = true;
    return true;
}

void AimbotFeature::shutdown() {
    logger::debug("Shutting down Aimbot Feature");
    m_input_manager = nullptr;
    BaseFeature::shutdown();
}

Vector3 AimbotFeature::get_bone_position(const Player& player, AimbotBone bone) const {
    switch (bone) {
        case AimbotBone::HEAD:
            return player.GetBone(BONE_DEF::HEAD).position;
        case AimbotBone::NECK:
            return player.GetBone(BONE_DEF::NECK).position;
        case AimbotBone::CHEST:
            return player.GetBone(BONE_DEF::SPINE).position;
        case AimbotBone::SPINE:
            return player.GetBone(BONE_DEF::SPINE1).position;
        case AimbotBone::HIP:
            return player.GetBone(BONE_DEF::HIP).position;
        default:
            return player.GetBone(BONE_DEF::HEAD).position;
    }
}

std::vector<Vector3> AimbotFeature::get_target_bones(const Player& player) const {
    std::vector<Vector3> target_bones;
    
    if (m_settings.multi_bone_enabled) {
        // Multi-bone targeting
        if (m_settings.target_head) {
            Vector3 bone_pos = get_bone_position(player, AimbotBone::HEAD);
            if (bone_pos != Vector3(0, 0, 0)) {
                target_bones.push_back(bone_pos);
            }
        }
        if (m_settings.target_neck) {
            Vector3 bone_pos = get_bone_position(player, AimbotBone::NECK);
            if (bone_pos != Vector3(0, 0, 0)) {
                target_bones.push_back(bone_pos);
            }
        }
        if (m_settings.target_chest) {
            Vector3 bone_pos = get_bone_position(player, AimbotBone::CHEST);
            if (bone_pos != Vector3(0, 0, 0)) {
                target_bones.push_back(bone_pos);
            }
        }
        if (m_settings.target_spine) {
            Vector3 bone_pos = get_bone_position(player, AimbotBone::SPINE);
            if (bone_pos != Vector3(0, 0, 0)) {
                target_bones.push_back(bone_pos);
            }
        }
        if (m_settings.target_hip) {
            Vector3 bone_pos = get_bone_position(player, AimbotBone::HIP);
            if (bone_pos != Vector3(0, 0, 0)) {
                target_bones.push_back(bone_pos);
            }
        }
    } else {
        // Single bone targeting
        Vector3 bone_pos = get_bone_position(player, m_settings.target_bone);
        if (bone_pos != Vector3(0, 0, 0)) {
            target_bones.push_back(bone_pos);
        }
    }
    
    return target_bones;
}

bool AimbotFeature::is_target_visible(const Vector3& local_eye_pos, const Vector3& target_pos) const {
    if (!m_settings.visible_only) {
        return true; // Skip visibility check if disabled
    }
    
    auto traceline_manager = m_engine->get_traceline_manager();
    if (!traceline_manager) {
        return true; // Assume visible if traceline manager is not available
    }
    
    return traceline_manager->is_visible(local_eye_pos, target_pos);
}

void AimbotFeature::update() {
    if (!is_feature_enabled() || !m_initialized || !m_entity_cache) {
        return;
    }
    
    // Reset target state
    m_has_target = false;
    m_current_target_distance = 0.0f;
    
    // Get all players
    auto& players = m_entity_cache->get_players();
    if (players.empty()) {
        return;
    }
    
    // Get local player for team check and eye position
    Player* local_player = m_entity_cache->get_local_player();
    if (!local_player) {
        return;
    }
    
    Vector3 local_eye_pos = local_player->GetBone(BONE_DEF::HEAD).position;
    if (local_eye_pos == Vector3(0, 0, 0)) {
        return;
    }
    
    Vector2 screen_center = m_renderer->get_screen_center();
    float closest_distance = FLT_MAX;
    Vector2 closest_target;
    bool found_target = false;
    
    // Loop through all players
    for (const auto& player : players) {
        // Skip if player is dead or invalid
        if (player.health <= 0 || player.origin == Vector3(0, 0, 0)) {
            continue;
        }
        
        // Skip local player
        if (player.instance == local_player->instance) {
            continue;
        }
        
        // Skip teammates if team check is enabled
        if (m_settings.team_check && player.team == local_player->team) {
            continue;
        }
        
        // Check world distance first
        float world_distance = player.origin.distance_to(local_player->origin) * 0.1f;
        if (world_distance > m_settings.max_distance) {
            continue;
        }
        
        // Get all possible target bones for this player
        std::vector<Vector3> target_bones = get_target_bones(player);
        if (target_bones.empty()) {
            continue;
        }
        
        // Find the best bone to target
        for (const Vector3& bone_pos : target_bones) {
            // Check visibility if required
            if (!is_target_visible(local_eye_pos, bone_pos)) {
                continue;
            }
            
            // Convert to screen position
            Vector2 screen_pos;
            if (!m_engine->world_to_screen(bone_pos, screen_pos)) {
                continue;
            }
            
            // Calculate distance from screen center
            Vector2 delta = screen_pos - screen_center;
            float screen_distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            
            // Check if within FOV
            if (screen_distance > m_settings.fov) {
                continue;
            }
            
            // Check if this is the closest target
            if (screen_distance < closest_distance) {
                closest_distance = screen_distance;
                closest_target = screen_pos;
                m_current_target_distance = world_distance;
                found_target = true;
            }
        }
    }
    
    if (found_target) {
        m_target_position = closest_target;
        m_has_target = true;
        logger::debug("Aimbot target found at: (" + std::to_string(m_target_position.x) + ", " + std::to_string(m_target_position.y) + ") distance: " + std::to_string(closest_distance) + "px, world: " + std::to_string(m_current_target_distance) + "m");
    }
}

void AimbotFeature::render() {
    if (!is_feature_enabled() || !m_initialized) {
        return;
    }
    
    // Draw FOV circle
    if (m_settings.show_fov_circle) {
        m_renderer->draw_circle(m_renderer->get_screen_center(), m_settings.fov, m_settings.fov_circle_color, 0, 2.0f);
    }
    
    // Draw target indicator and info
    if (m_has_target) {
        // Draw target indicator
        m_renderer->draw_circle(m_target_position, 8.0f, ImColor(255, 0, 0, 255), 0, 2.0f);
        m_renderer->draw_circle(m_target_position, 4.0f, ImColor(255, 255, 255, 255), 0, 1.0f);
        
        // Draw target info
        if (m_settings.show_target_info) {
            std::string target_info = "TARGET";
            if (m_current_target_distance > 0) {
                target_info += " [" + std::to_string(static_cast<int>(m_current_target_distance)) + "m]";
            }
            m_renderer->draw_text(m_target_position + Vector2(15, -5), target_info, ImColor(255, 255, 255, 255));
        }
        
        // Draw aim line when aiming
        if (m_settings.show_aim_line && should_aim()) {
            Vector2 center = m_renderer->get_screen_center();
            m_renderer->draw_line(center, m_target_position, ImColor(0, 255, 0, 180), 1.5f);
        }
    }
}

void AimbotFeature::process_input() {
    if (!is_feature_enabled() || !m_initialized || !m_input_manager) {
        return;
    }
    
    // Check if we should aim and have a target
    if (should_aim() && m_has_target) {
        aim_at_screen_position(m_target_position);
    }
}

bool AimbotFeature::should_aim() const {
    if (!m_settings.aim_key_enabled) {
        return true; // Always aim if aim key is disabled
    }
    
    return m_input_manager->is_key_down(m_settings.aim_key);
}

void AimbotFeature::aim_at_screen_position(const Vector2& target_pos) {
    Vector2 screen_center = m_renderer->get_screen_center();
    
    float target_x = 0;
    float target_y = 0;
    
    // Calculate X movement
    if (target_pos.x > screen_center.x) {
        target_x = -(screen_center.x - target_pos.x);
        if (target_x + screen_center.x > screen_center.x * 2) target_x = 0;
    } else {
        target_x = target_pos.x - screen_center.x;
        if (target_x + screen_center.x < 0) target_x = 0;
    }
    
    // Calculate Y movement
    if (target_pos.y > screen_center.y) {
        target_y = -(screen_center.y - target_pos.y);
        if (target_y + screen_center.y > screen_center.y * 2) target_y = 0;
    } else {
        target_y = target_pos.y - screen_center.y;
        if (target_y + screen_center.y < 0) target_y = 0;
    }
    
    // Apply smoothing
    float smoothing_factor = m_settings.smooth + 5.0f;
    target_x /= smoothing_factor;
    target_y /= smoothing_factor;
    
    // Move the mouse
    if (target_x != 0 || target_y != 0) {
        m_input_manager->move_mouse(static_cast<int>(target_x), static_cast<int>(target_y));
        logger::debug("Aimbot: Moving mouse (" + std::to_string(target_x) + ", " + std::to_string(target_y) + ") smooth: " + std::to_string(smoothing_factor));
    }
}
