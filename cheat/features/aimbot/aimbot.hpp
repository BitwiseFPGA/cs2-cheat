#pragma once
#include <logger/logger.hpp>
#include <engine/sdk/math/vector.hpp>
#include <engine/sdk/types/impl/player_impl.hpp>
#include <features/base_feature.hpp>
#include <features/aimbot/settings/aimbot_settings.hpp>
#include <memory>
#include <vector>

class EntityCache;
class WorldCache;
class SettingsManager;
class Renderer;

class AimbotFeature : public BaseFeature {
public:
    AimbotFeature(EntityCache* entity_cache, WorldCache* world_cache, Renderer* renderer);
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
