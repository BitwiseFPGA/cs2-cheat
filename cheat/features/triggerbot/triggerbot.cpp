#include <features/triggerbot/triggerbot.hpp>
#include <engine/cache/entity/entity_cache.hpp>
#include <engine/cache/world/world_cache.hpp>
#include <renderer/renderer.hpp>
#include <input/input.hpp>
#include <input/adapter/base_input.h>
#include <engine/engine.hpp>
#include <engine/sdk/types/player_impl.hpp>
#include <engine/physics/traceline.hpp>
#include <logger/logger.hpp>
#include <algorithm>
#include <cmath>
#include <cfloat>

TriggerbotFeature::TriggerbotFeature(EntityCache* entity_cache, WorldCache* world_cache, Renderer* renderer, Engine* engine)
    : BaseFeature("Triggerbot", entity_cache, world_cache, renderer, engine)
    , m_input_manager(nullptr)
    , m_target_in_crosshair(false)
    , m_target_position(0, 0, 0)
    , m_target_distance(0.0f)
    , m_target_info("")
    , m_should_trigger(false)
    , m_trigger_queued(false)
    , m_gen(m_rd())
    , m_delay_dist(0.0f, 1.0f)
{
    auto now = std::chrono::high_resolution_clock::now();
    m_trigger_time = now;
    m_last_shot_time = now;
    logger::debug("Triggerbot Feature created");
}

TriggerbotFeature::~TriggerbotFeature() {
    logger::debug("Triggerbot Feature destroyed");
}

bool TriggerbotFeature::initialize() {
    logger::debug("Initializing Triggerbot Feature");
    
    if (m_engine) {
        m_input_manager = m_engine->get_input_manager();
    }
    
    if (!m_input_manager) {
        logger::error("Failed to get InputManager for Triggerbot");
        return false;
    }
    
    m_initialized = true;
    return true;
}

void TriggerbotFeature::shutdown() {
    logger::debug("Shutting down Triggerbot Feature");
    m_input_manager = nullptr;
    BaseFeature::shutdown();
}

Vector3 TriggerbotFeature::get_target_bone_position(const Player& player) const {
    if (m_settings.head_only) {
        return player.GetBone(BONE_DEF::HEAD).position;
    }
    
    // Default to chest for body shots
    return player.GetBone(BONE_DEF::SPINE).position;
}

bool TriggerbotFeature::is_target_visible(const Vector3& local_eye_pos, const Vector3& target_pos) const {
    if (!m_settings.visible_only) {
        return true;
    }
    
    auto traceline_manager = m_engine->get_traceline_manager();
    if (!traceline_manager) {
        return true;
    }
    
    return traceline_manager->is_visible(local_eye_pos, target_pos);
}

bool TriggerbotFeature::is_crosshair_on_target(const Vector3& target_pos, Vector2& screen_pos) const {
    if (!m_engine->world_to_screen(target_pos, screen_pos)) {
        return false;
    }
    
    Vector2 screen_center = m_renderer->get_screen_center();
    float distance_from_center = std::sqrtf(
        std::pow(screen_pos.x - screen_center.x, 2) + 
        std::pow(screen_pos.y - screen_center.y, 2)
    );
    
    return distance_from_center <= m_settings.crosshair_tolerance;
}

bool TriggerbotFeature::is_target_valid(const Player& player, const Player& local_player) const {
    // Basic validity checks
    if (player.health <= 0 || player.origin == Vector3(0, 0, 0)) {
        return false;
    }
    
    // Don't target self
    if (player.instance == local_player.instance) {
        return false;
    }
    
    // Team check
    if (m_settings.team_check && player.team == local_player.team) {
        return false;
    }
    
    // Distance check
    float world_distance = player.origin.distance_to(local_player.origin) * 0.1f; // Convert to meters
    if (world_distance > m_settings.max_distance) {
        return false;
    }
    
    return true;
}

bool TriggerbotFeature::check_weapon_compatibility() const {
    // For now, we'll assume all weapons are compatible
    // This can be expanded later to check actual weapon types
    return (m_settings.rifle_enabled || m_settings.pistol_enabled || 
            m_settings.sniper_enabled || m_settings.smg_enabled);
}

bool TriggerbotFeature::check_safety_conditions(const Player& local_player) const {
    // Check if player is flashed (simplified check)
    if (m_settings.flash_check) {
        // This would need actual flash detection implementation
        // For now, we'll skip this check
    }
    
    // Check if in smoke (simplified check)
    if (m_settings.smoke_check) {
        // This would need actual smoke detection implementation
        // For now, we'll skip this check
    }
    
    // Check if scoped when auto_scope_only is enabled
    if (m_settings.auto_scope_only) {
        // This would need actual scope detection implementation
        // For now, we'll assume always scoped
    }
    
    return true;
}

float TriggerbotFeature::generate_random_delay() const {
    if (m_settings.trigger_delay_min >= m_settings.trigger_delay_max) {
        return m_settings.trigger_delay_min;
    }
    
    float random_factor = m_delay_dist(m_gen);
    return m_settings.trigger_delay_min + 
           (m_settings.trigger_delay_max - m_settings.trigger_delay_min) * random_factor;
}

void TriggerbotFeature::queue_trigger_shot() {
    if (m_trigger_queued) {
        return;
    }
    
    float delay_ms = generate_random_delay();
    m_trigger_time = std::chrono::high_resolution_clock::now() + 
                    std::chrono::milliseconds(static_cast<int>(delay_ms));
    m_trigger_queued = true;
    
    logger::debug("Triggerbot: Shot queued with delay: " + std::to_string(delay_ms) + "ms");
}

void TriggerbotFeature::execute_trigger_shot() {
    if (!m_input_manager) {
        return;
    }
    
    // Check minimum interval between shots
    auto now = std::chrono::high_resolution_clock::now();
    auto time_since_last_shot = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_shot_time);
    
    if (time_since_last_shot.count() < MIN_SHOT_INTERVAL_MS) {
        return;
    }
    
    // Simulate mouse click (left mouse button)
    m_input_manager->click_mouse_button(MouseButton::Left);
    
    m_last_shot_time = now;
    m_trigger_queued = false;
    
    logger::debug("Triggerbot: Shot executed");
}

void TriggerbotFeature::update() {
    if (!is_feature_enabled() || !m_initialized || !m_entity_cache) {
        return;
    }
    
    // Reset state
    m_target_in_crosshair = false;
    m_should_trigger = false;
    m_target_info = "";
    
    auto& players = m_entity_cache->get_players();
    if (players.empty()) {
        return;
    }
    
    Player* local_player = m_entity_cache->get_local_player();
    if (!local_player) {
        return;
    }
    
    // Check safety conditions
    if (!check_safety_conditions(*local_player)) {
        return;
    }
    
    // Check weapon compatibility
    if (!check_weapon_compatibility()) {
        return;
    }
    
    Vector3 local_eye_pos = local_player->GetBone(BONE_DEF::HEAD).position;
    if (local_eye_pos == Vector3(0, 0, 0)) {
        return;
    }
    
    // Check for targets in crosshair
    for (const auto& player : players) {
        if (!is_target_valid(player, *local_player)) {
            continue;
        }
        
        Vector3 target_bone_pos = get_target_bone_position(player);
        if (target_bone_pos == Vector3(0, 0, 0)) {
            continue;
        }
        
        // Check visibility
        if (!is_target_visible(local_eye_pos, target_bone_pos)) {
            continue;
        }
        
        // Check if target is in crosshair
        Vector2 screen_pos;
        if (is_crosshair_on_target(target_bone_pos, screen_pos)) {
            m_target_in_crosshair = true;
            m_target_position = target_bone_pos;
            m_target_distance = player.origin.distance_to(local_player->origin) * 0.1f;
            m_target_info = "TARGET [" + std::to_string(static_cast<int>(m_target_distance)) + "m]";
            m_should_trigger = true;
            
            // Queue shot if conditions are met
            if (should_trigger()) {
                queue_trigger_shot();
            }
            
            break; // Only target one player at a time
        }
    }
}

void TriggerbotFeature::render() {
    if (!is_feature_enabled() || !m_initialized) {
        return;
    }
    
    Vector2 screen_center = m_renderer->get_screen_center();
    
    // Show crosshair circle
    if (m_settings.show_crosshair_circle) {
        m_renderer->draw_circle(screen_center, m_settings.crosshair_tolerance, 
                               m_settings.crosshair_color, 0, 1.5f);
    }
    
    // Show trigger indicator
    if (m_settings.show_trigger_indicator && m_target_in_crosshair) {
        ImColor indicator_color = m_trigger_queued ? ImColor(255, 165, 0, 255) : ImColor(255, 0, 0, 255);
        m_renderer->draw_circle(screen_center, 3.0f, indicator_color, 0, 2.0f);
    }
    
    // Show target info
    if (m_settings.show_target_info && m_target_in_crosshair && !m_target_info.empty()) {
        m_renderer->draw_text(screen_center + Vector2(20, -10), m_target_info, ImColor(255, 255, 255, 255));
    }
    
    // Show status when triggerbot is active
    if (should_trigger()) {
        std::string status = "TRIGGERBOT ACTIVE";
        if (m_trigger_queued) {
            status += " (QUEUED)";
        }
        m_renderer->draw_text(Vector2(10, 50), status, ImColor(0, 255, 0, 255));
    }
}

void TriggerbotFeature::process_input() {
    if (!is_feature_enabled() || !m_initialized || !m_input_manager) {
        return;
    }
    
    // Execute queued shots
    if (m_trigger_queued) {
        auto now = std::chrono::high_resolution_clock::now();
        if (now >= m_trigger_time) {
            // Double-check that we still have a valid target before shooting
            if (m_target_in_crosshair && should_trigger()) {
                execute_trigger_shot();
            } else {
                // Cancel the queued shot if target is no longer valid
                m_trigger_queued = false;
                logger::debug("Triggerbot: Shot cancelled - target no longer valid");
            }
        }
    }
}

bool TriggerbotFeature::should_trigger() const {
    if (!m_settings.trigger_key_enabled) {
        return true; // Always trigger when no key is required
    }
    
    return m_input_manager->is_key_down(m_settings.trigger_key);
} 