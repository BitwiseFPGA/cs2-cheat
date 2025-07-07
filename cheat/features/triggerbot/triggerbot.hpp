#pragma once
#include <logger/logger.hpp>
#include <engine/sdk/math/vector.hpp>
#include <features/base_feature.hpp>
#include <features/triggerbot/settings/triggerbot_settings.hpp>
#include <input/adapter/base_input.h>
#include <memory>
#include <chrono>
#include <random>

class EntityCache;
class WorldCache;
class SettingsManager;
class Renderer;
class Engine;
class InputManager;
class Player;

class TriggerbotFeature : public BaseFeature {
public:
    TriggerbotFeature(EntityCache* entity_cache, WorldCache* world_cache, Renderer* renderer, Engine* engine);
    ~TriggerbotFeature();
    
    bool initialize() override;
    void shutdown() override;
    void update() override;
    void render() override;
    void process_input() override;
    
    TriggerbotSettings& get_settings() { return m_settings; }
    const TriggerbotSettings& get_settings() const { return m_settings; }
    
    bool is_feature_enabled() const override { return m_settings.enabled; }
    
private:
    TriggerbotSettings m_settings;
    InputManager* m_input_manager;
}; 