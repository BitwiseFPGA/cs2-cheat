#pragma once
#include <renderer/renderer.hpp>
#include <logger/logger.hpp>

#include <string>
#include <memory>

class EntityCache;
class WorldCache;
class Engine;

class BaseFeature {
public:
    BaseFeature(const std::string& feature_name, EntityCache* entity_cache = nullptr, WorldCache* world_cache = nullptr, Renderer* renderer = nullptr, Engine* engine = nullptr);
    virtual ~BaseFeature();
    
    virtual bool initialize() = 0;
    virtual void shutdown();
    virtual void update() = 0;
    
    virtual void render() {}
    virtual void process_input() {}
    
    virtual void set_enabled(bool enabled) { m_enabled = enabled; }
    virtual bool is_enabled() const { return m_enabled; }
    
    virtual bool is_feature_enabled() const { return m_enabled; }
    virtual bool is_initialized() const { return m_initialized; }
    
    const std::string& get_name() const { return m_feature_name; }
    
    EntityCache* get_entity_cache() const { return m_entity_cache; }
    WorldCache* get_world_cache() const { return m_world_cache; }
    Renderer* get_renderer() const { return m_renderer; }
    Engine* get_engine() const { return m_engine; }
    
protected:
    std::string m_feature_name;
    bool m_initialized;
    bool m_enabled;
    
    EntityCache* m_entity_cache;
    WorldCache* m_world_cache;
    Renderer* m_renderer;
    Engine* m_engine;
};

using FeaturePtr = std::shared_ptr<BaseFeature>; 