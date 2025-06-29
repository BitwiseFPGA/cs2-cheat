#pragma once
#include <logger/logger.hpp>
#include <engine/sdk/math/vector.hpp>
#include <features/base_feature.hpp>
#include <features/esp/settings/esp_settings.hpp>
#include <memory>
#include <vector>
#include <string>

class EntityCache;
class WorldCache;
class Renderer;
class Engine;

class EspFeature : public BaseFeature {
public:
    EspFeature(EntityCache* entity_cache, WorldCache* world_cache, Renderer* renderer, Engine* engine);
    ~EspFeature();
    
    bool initialize() override;
    void shutdown() override;
    void update() override;
    void render() override;
    
    EspSettings& get_settings() { return m_settings; }
    const EspSettings& get_settings() const { return m_settings; }
    
    bool is_feature_enabled() const override { return m_settings.enabled; }
    
private:
    EspSettings m_settings;
};

