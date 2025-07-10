#include <features/triggerbot/triggerbot.hpp>
#include <engine/cache/entity/entity_cache.hpp>
#include <engine/cache/world/world_cache.hpp>
#include <renderer/renderer.hpp>
#include <input/input.hpp>
#include <input/adapter/base_input.h>
#include <engine/engine.hpp>
#include <engine/sdk/types/player.hpp>
#include <engine/physics/traceline.hpp>
#include <logger/logger.hpp>

#include <algorithm>
#include <cmath>
#include <cfloat>

TriggerbotFeature::TriggerbotFeature(EntityCache* entity_cache, WorldCache* world_cache, Renderer* renderer, Engine* engine)
    : BaseFeature("Triggerbot", entity_cache, world_cache, renderer, engine)
    , m_input_manager(nullptr)
    , m_random_generator(m_random_device())
{
    auto now = std::chrono::high_resolution_clock::now();
    m_last_crosshair_change = now;
    m_reaction_start_time = now;
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
    m_entity_tracking.clear();
    BaseFeature::shutdown();
}

void TriggerbotFeature::update() {
    if (!is_feature_enabled() || !m_initialized || !m_entity_cache) {
        return;
    }

    auto& players = m_entity_cache->get_players();
    if (players.empty()) {
        return;
    }

    Player* local_player = m_entity_cache->get_local_player();
    if (!local_player) {
        return;
    }

    uintptr_t crosshair_ent = m_entity_cache->get_crosshair_entity();
    
    // Check if crosshair entity changed
    if (crosshair_ent != m_last_crosshair_entity) {
        m_last_crosshair_entity = crosshair_ent;
        m_last_crosshair_change = std::chrono::high_resolution_clock::now();
        m_reaction_timer_active = (crosshair_ent != 0);
        if (m_reaction_timer_active) {
            m_current_reaction_delay = GetRandomReactionTime();
            m_reaction_start_time = m_last_crosshair_change;
        }
    }
    
    if (crosshair_ent == 0) {
        return;
    }

    // Check if we should use trigger key
    if (m_settings.trigger_key_enabled && m_input_manager) {
        if (!m_input_manager->is_key_pressed(m_settings.trigger_key)) {
            return;
        }
    }

    // Check reaction time
    if (!IsReactionTimeReady()) {
        return;
    }

    for (const auto& player : players) {
        if (player.health <= 0 || player.origin == Vector3(0, 0, 0))
            continue;

        if (player.instance == local_player->instance)
            continue;

        if (player.team == local_player->team)
            continue;

        if (player.base_entity == crosshair_ent) {
            if (CanShootAtEntity(crosshair_ent)) {
                Shoot();
                UpdateEntityTracking(crosshair_ent);
            }
            break;
        }
    }
}

void TriggerbotFeature::render() {
    if (!is_feature_enabled() || !m_initialized) {
        return;
    }
}

void TriggerbotFeature::process_input() {
    if (!is_feature_enabled() || !m_initialized || !m_input_manager) {
        return;
    }
}

void TriggerbotFeature::Shoot()
{
    m_input_manager->click_mouse_button(MouseButton::Left);
}

bool TriggerbotFeature::CanShootAtEntity(uintptr_t entity_id) {
    auto now = std::chrono::high_resolution_clock::now();
    
    // Find or create tracking data for this entity
    auto& tracking = m_entity_tracking[entity_id];
    
    // Check if entity is in cooldown
    if (tracking.in_cooldown) {
        auto cooldown_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - tracking.cooldown_start_time).count();
        
        if (cooldown_duration >= m_settings.burst_cooldown_ms) {
            // Cooldown expired, reset tracking
            tracking.in_cooldown = false;
            tracking.shots_fired = 0;
        } else {
            // Still in cooldown
            return false;
        }
    }
    
    // Check if we've reached burst limit
    if (tracking.shots_fired >= m_settings.burst_shots) {
        // Start cooldown
        tracking.in_cooldown = true;
        tracking.cooldown_start_time = now;
        return false;
    }
    
    return true;
}

bool TriggerbotFeature::IsReactionTimeReady() {
    if (!m_reaction_timer_active) {
        return true;
    }
    
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_reaction_start_time).count();
    
    return elapsed >= m_current_reaction_delay;
}

void TriggerbotFeature::UpdateEntityTracking(uintptr_t entity_id) {
    auto now = std::chrono::high_resolution_clock::now();
    auto& tracking = m_entity_tracking[entity_id];
    
    tracking.shots_fired++;
    tracking.last_shot_time = now;
    
    // Reset reaction timer after shooting
    m_reaction_timer_active = true;
    m_current_reaction_delay = GetRandomReactionTime();
    m_reaction_start_time = now;
}

int TriggerbotFeature::GetRandomReactionTime() {
    std::uniform_int_distribution<int> distribution(
        m_settings.reaction_time_min_ms, 
        m_settings.reaction_time_max_ms
    );
    return distribution(m_random_generator);
}
