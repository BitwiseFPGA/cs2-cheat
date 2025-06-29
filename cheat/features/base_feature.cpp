#include <features/base_feature.hpp>

BaseFeature::BaseFeature(const std::string& feature_name, EntityCache* entity_cache, WorldCache* world_cache, Renderer* renderer, Engine* engine)
    : m_feature_name(feature_name)
    , m_initialized(false)
    , m_enabled(false)
    , m_entity_cache(entity_cache)
    , m_world_cache(world_cache)
    , m_renderer(renderer)
    , m_engine(engine)
{
}

BaseFeature::~BaseFeature() {
    if (m_initialized) {
        shutdown();
    }
}

void BaseFeature::shutdown() {
    m_enabled = false;
    m_initialized = false;
}