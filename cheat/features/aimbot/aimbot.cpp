#include <features/aimbot/aimbot.hpp>
#include <engine/cache/entity/entity_cache.hpp>
#include <engine/cache/world/world_cache.hpp>
#include <engine/renderer/renderer.hpp>
#include <logger/logger.hpp>
#include <algorithm>
#include <cmath>

AimbotFeature::AimbotFeature(EntityCache* entity_cache, WorldCache* world_cache, Renderer* renderer)
    : BaseFeature("Aimbot", entity_cache, world_cache, renderer)
{
    logger::debug("Aimbot Feature created");
}

AimbotFeature::~AimbotFeature() {
    logger::debug("Aimbot Feature destroyed");
}

bool AimbotFeature::initialize() {
    logger::debug("Initializing Aimbot Feature");
    m_initialized = true;
    return true;
}

void AimbotFeature::shutdown() {
    logger::debug("Shutting down Aimbot Feature");
    BaseFeature::shutdown();
}

void AimbotFeature::update() {
    if (!is_feature_enabled() || !m_initialized) {
        return;
    }
    
    logger::debug("Updating Aimbot Feature");
}

void AimbotFeature::render() {
    if (!is_feature_enabled() || !m_initialized) {
        return;
    }
    
    if (m_settings.show_fov_circle) {
        m_renderer->draw_circle(m_renderer->get_screen_center(), m_settings.fov, m_settings.fov_circle_color, 0, 3.0f);
    }
}
