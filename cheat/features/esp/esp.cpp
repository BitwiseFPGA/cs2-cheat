#include <features/esp/esp.hpp>
#include <engine/cache/entity/entity_cache.hpp>
#include <engine/cache/world/world_cache.hpp>
#include <engine/renderer/renderer.hpp>
#include <engine/engine.hpp>
#include <logger/logger.hpp>
#include <algorithm>

EspFeature::EspFeature(EntityCache* entity_cache, WorldCache* world_cache, Renderer* renderer, Engine* engine)
    : BaseFeature("ESP", entity_cache, world_cache, renderer, engine)
{
    logger::debug("ESP Feature created");
}

EspFeature::~EspFeature() {
    logger::debug("ESP Feature destroyed");
}

bool EspFeature::initialize() {
    logger::debug("Initializing ESP Feature");
    m_initialized = true;
    return true;
}

void EspFeature::shutdown() {
    logger::debug("Shutting down ESP Feature");
    BaseFeature::shutdown();
}

void EspFeature::update() {
    if (!is_feature_enabled() || !m_initialized) {
        return;
    }
}

void EspFeature::render() {
    if (!is_feature_enabled() || !m_initialized || !m_renderer || !m_engine || !m_entity_cache) {
        return;
    }
    
    const auto& players = m_entity_cache->get_players();
    
    for (const auto& player : players) {
        if (player.health <= 0) {
            continue;
        }
        
        Vector2 screen_pos;
        if (m_engine->world_to_screen(player.origin, screen_pos)) {
            m_renderer->draw_filled_circle(screen_pos, 5.0f, m_settings.box_color, 16);
        }
    }
}
