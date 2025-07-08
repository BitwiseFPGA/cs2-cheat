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
    
    void RenderPlayers();
    void RenderWorldEntities();
    void RenderSmokes();
    void RenderMapTriangles();
    
    bool IsWeaponEntity(uint32_t classname_hash) const;
    bool IsGrenadeEntity(uint32_t classname_hash) const;
    bool IsHostageEntity(uint32_t classname_hash) const;
    bool IsChickenEntity(uint32_t classname_hash) const;
    ImColor GetEntityColor(uint32_t classname_hash) const;
    std::string GetEntityDisplayName(uint32_t classname_hash) const;
};

