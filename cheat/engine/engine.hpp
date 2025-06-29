#pragma once
#include <logger/logger.hpp>
#include <features/base_feature.hpp>
#include <engine/sdk/math/matrix.hpp>
#include <memory>
#include <string>
#include <vector>
#include <chrono>

class AccessManager;
class EntityCache;
class WorldCache;
class Renderer;
class TracelineManager;
class MainMenu;
class SettingsManager;

class Engine {
public:
    Engine();
    ~Engine();
    
    bool initialize();
    void shutdown();
    
    void run();
    bool is_running() const { return m_running; }
    void stop() { m_running = false; }
    
    AccessManager* get_access_manager() const { return m_access_manager.get(); }
    EntityCache* get_entity_cache() const { return m_entity_cache.get(); }
    WorldCache* get_world_cache() const { return m_world_cache.get(); }
    Renderer* get_renderer() const { return m_renderer.get(); }
    TracelineManager* get_traceline_manager() const { return m_traceline_manager.get(); }
    MainMenu* get_menu() const { return m_main_menu.get(); }
    SettingsManager* get_settings_manager() const { return m_settings_manager.get(); }
    
    void add_feature(FeaturePtr feature);
    template<typename T, typename... Args>
    void add_feature(Args&&... args) {
        auto feature = std::make_shared<T>(std::forward<Args>(args)...);
        add_feature(feature);
    }
    const std::vector<FeaturePtr>& get_features() const { return m_features; }
    
    template<typename T>
    std::shared_ptr<T> get_feature() const {
        for (const auto& feature : m_features) {
            auto typed_feature = std::dynamic_pointer_cast<T>(feature);
            if (typed_feature) {
                return typed_feature;
            }
        }
        return nullptr;
    }
    
    FeaturePtr get_feature_by_name(const std::string& name) const {
        for (const auto& feature : m_features) {
            if (feature && feature->get_name() == name) {
                return feature;
            }
        }
        return nullptr;
    }
    
    const Matrix4x4& get_view_matrix() const { return m_view_matrix; }

    bool world_to_screen(const Vector3& world_pos, Vector2& screen_pos) const;

private:
    bool m_running;
    bool m_initialized;
    
    std::unique_ptr<AccessManager> m_access_manager;
    std::unique_ptr<EntityCache> m_entity_cache;
    std::unique_ptr<WorldCache> m_world_cache;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<TracelineManager> m_traceline_manager;
    
    std::unique_ptr<MainMenu> m_main_menu;
    std::unique_ptr<SettingsManager> m_settings_manager;
    
    std::vector<FeaturePtr> m_features;
    
    static constexpr int MAX_UPDATES_PER_FRAME = 2;
    static constexpr double CACHE_UPDATE_INTERVAL_MS = 1000.0;
    static constexpr double FRAME_UPDATE_INTERVAL_MS = 10.0;
    static constexpr double PHYSICS_UPDATE_INTERVAL_MS = 1000.0;
    static constexpr double FEATURES_UPDATE_INTERVAL_MS = 100.0;
    
    std::chrono::high_resolution_clock::time_point m_last_cache_update;
    std::chrono::high_resolution_clock::time_point m_last_frame_update;
    std::chrono::high_resolution_clock::time_point m_last_physics_update;
    std::chrono::high_resolution_clock::time_point m_last_features_update;
    int m_current_frame_updates;

    Matrix4x4 m_view_matrix = {};
    
    bool should_update_system(std::chrono::high_resolution_clock::time_point& last_update, double interval_ms);
    
    bool initialize_access_system();
    bool initialize_cache_system();
    bool initialize_physics_system();
    bool initialize_renderer();
    bool initialize_menu_and_settings();
    bool initialize_features();
    
    void update_caches();
    void update_view();
    void update_physics();
    void update_features();
    void render_features();
    void process_input();
};