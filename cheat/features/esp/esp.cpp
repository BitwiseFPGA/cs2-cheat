#include <features/esp/esp.hpp>
#include <engine/cache/entity/entity_cache.hpp>
#include <engine/cache/world/world_cache.hpp>
#include <engine/renderer/renderer.hpp>
#include <logger/logger.hpp>
#include <algorithm>

EspFeature::EspFeature(EntityCache* entity_cache, WorldCache* world_cache, Renderer* renderer)
    : BaseFeature("ESP", entity_cache, world_cache, renderer)
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
    
    logger::debug("Updating ESP Feature");
}

void EspFeature::render() {
    if (!is_feature_enabled() || !m_initialized || !m_renderer) {
        return;
    }
        
    m_renderer->draw_rect(m_renderer->get_screen_center(), Vector2(100, 100), m_settings.box_color, 1.0f);
}
