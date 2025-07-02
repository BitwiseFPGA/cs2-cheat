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

class AimbotFeature : public BaseFeature {
public:
    AimbotFeature(EntityCache* entity_cache, WorldCache* world_cache, Renderer* renderer, Engine* engine);
    ~AimbotFeature();
    
    bool initialize() override;
    void shutdown() override;
    void update() override;
    void render() override;
    
    AimbotSettings& get_settings() { return m_settings; }
    const AimbotSettings& get_settings() const { return m_settings; }
    
    bool is_feature_enabled() const override { return m_settings.enabled; }
    
private:
    AimbotSettings m_settings;
};
