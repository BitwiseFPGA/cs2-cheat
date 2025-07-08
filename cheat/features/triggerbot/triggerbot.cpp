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
{
    auto now = std::chrono::high_resolution_clock::now();
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

void TriggerbotFeature::update() {
    if (!is_feature_enabled() || !m_initialized || !m_entity_cache) {
        return;
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
