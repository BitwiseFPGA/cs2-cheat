#include <engine/engine.hpp>
#include <access/access.hpp>
#include <engine/cache/entity/entity_cache.hpp>
#include <engine/cache/world/world_cache.hpp>
#include <engine/physics/traceline.hpp>
#include <engine/renderer/adapters/d3d11.hpp>
#include <features/aimbot/aimbot.hpp>
#include <features/esp/esp.hpp>
#include <config/runtime/settings.hpp>
#include <menu/main_menu.hpp>
#include <thread>
#include <chrono>
#include <windows.h>
#include "sdk/offsets/static/offsets.hpp"

Engine::Engine() 
    : m_running(false)
    , m_initialized(false)
    , m_access_manager(nullptr)
    , m_entity_cache(nullptr)
    , m_world_cache(nullptr)
    , m_renderer(nullptr)
    , m_traceline_manager(nullptr)
    , m_main_menu(nullptr)
    , m_settings_manager(nullptr)
    , m_current_frame_updates(0)
{
    auto now = std::chrono::high_resolution_clock::now();
    m_last_cache_update = now;
    m_last_physics_update = now;
    m_last_features_update = now;
}

Engine::~Engine() {
    if (m_initialized) {
        shutdown();
    }
    logger::info("Engine destructor called");
}

bool Engine::initialize() {    
    try {
        logger::log_step("1/6", "Setting up access system");
        if (!initialize_access_system()) {
            logger::log_failure("Access System");
            return false;
        }
        
        logger::log_step("2/6", "Setting up entity and world caches");
        if (!initialize_cache_system()) {
            logger::log_failure("Cache System");
            return false;
        }
        
        logger::log_step("3/6", "Setting up physics and traceline system");
        if (!initialize_physics_system()) {
            logger::log_failure("Physics System");
            return false;
        }

        logger::log_step("4/6", "Setting up DirectX 11 renderer");
        if (!initialize_renderer()) {
            logger::log_failure("Renderer System");
            return false;
        }

        logger::log_step("5/6", "Setting up menu and settings system");
        if (!initialize_menu_and_settings()) {
            logger::log_failure("Menu and Settings System");
            return false;
        }

        logger::log_step("6/6", "Loading cheat features");
        if (!initialize_features()) {
            logger::log_failure("Feature System");
            return false;
        }
        
        m_initialized = true;
        logger::success("Engine fully initialized and ready");
        return true;
        
    } catch (const std::exception& e) {
        logger::critical("Engine initialization failed with exception: " + std::string(e.what()));
        return false;
    }
}

void Engine::shutdown() {
    logger::log_step("Engine Shutdown", "Beginning graceful shutdown");
    
    m_running = false;
    
    for (auto it = m_features.rbegin(); it != m_features.rend(); ++it) {
        if (*it && (*it)->is_initialized()) {
            (*it)->shutdown();
        }
    }
    m_features.clear();
    
    if (m_main_menu) {
        m_main_menu->shutdown();
        m_main_menu.reset();
    }
    
    if (m_settings_manager) {
        m_settings_manager.reset();
    }
    
    if (m_renderer) {
        m_renderer.reset();
    }
    
    if (m_traceline_manager) {
        m_traceline_manager.reset();
    }
    
    if (m_world_cache) {
        m_world_cache.reset();
    }
    
    if (m_entity_cache) {
        m_entity_cache.reset();
    }
    
    if (m_access_manager) {
        m_access_manager.reset();
    }
    
    m_initialized = false;
    logger::success("Engine shutdown completed");
}

void Engine::run() {
    if (!m_initialized) {
        logger::error("Cannot run engine - not initialized");
        return;
    }
    
    logger::info("Starting main engine loop");
    m_running = true;
    
    while (m_running) {
        try {
            m_current_frame_updates = 0;
            
            if (m_renderer) {
                m_renderer->begin_frame();
                m_renderer->begin_imgui_frame();
            }
            
            update_caches();

			update_view();
            
            update_physics();
            
            update_features();
            
            render_features();
            
            process_input();
            
            if (m_main_menu) {
                m_main_menu->render();
            }
            
            if (m_renderer) {
                m_renderer->render_imgui();
                m_renderer->end_imgui_frame();
                m_renderer->end_frame();
                m_renderer->present();
            }
            
        } catch (const std::exception& e) {
            logger::error("Exception in main loop: " + std::string(e.what()));
        }
    }
    
    logger::info("Main engine loop ended");
}

bool Engine::initialize_access_system() {    
    try {
        m_access_manager = std::make_unique<AccessManager>();
        if (!m_access_manager->initialize()) {
            logger::error("Failed to initialize AccessManager");
            return false;
        }
        
        return true;
    } catch (...) {
        logger::error("Failed to initialize access system");
        return false;
    }
}

bool Engine::initialize_cache_system() {    
    try {
        m_entity_cache = std::make_unique<EntityCache>(m_access_manager.get());
        m_entity_cache->initialize();
        
        m_world_cache = std::make_unique<WorldCache>(m_access_manager.get());
        m_world_cache->initialize();

        return true;
    } catch (...) {
        logger::error("Failed to initialize cache systems");
        return false;
    }
}

bool Engine::initialize_physics_system() {    
    try {
        m_traceline_manager = std::make_unique<TracelineManager>(m_world_cache.get());
        
        if (!m_traceline_manager->initialize()) {
            logger::error("Failed to initialize TraclineManager");
            return false;
        }
        return true;
        
    } catch (const std::exception& e) {
        logger::error("Failed to initialize physics system: " + std::string(e.what()));
        return false;
    }
}

bool Engine::initialize_renderer() {    
    try {
        m_renderer = std::make_unique<D3D11Renderer>();
        
        if (!m_renderer->initialize()) {
            logger::error("Failed to initialize D3D11 renderer");
            return false;
        }
        return true;
        
    } catch (const std::exception& e) {
        logger::error("Failed to initialize renderer: " + std::string(e.what()));
        return false;
    }
}

bool Engine::initialize_menu_and_settings() {
    try {
        m_settings_manager = std::make_unique<SettingsManager>();
        
        m_main_menu = std::make_unique<MainMenu>();
        if (!m_main_menu->initialize(m_renderer.get(), m_settings_manager.get())) {
            logger::error("Failed to initialize main menu");
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        logger::error("Failed to initialize menu and settings: " + std::string(e.what()));
        return false;
    }
}

bool Engine::initialize_features() {    
    try {
        add_feature<AimbotFeature>(m_entity_cache.get(), m_world_cache.get(), m_renderer.get(), this);
        add_feature<EspFeature>(m_entity_cache.get(), m_world_cache.get(), m_renderer.get(), this);
        
        for (auto& feature : m_features) {
            if (feature) {
                if (!feature->initialize()) {
                    logger::error("Failed to initialize feature: " + feature->get_name());
                    return false;
                }
            }
        }
        
        for (auto& feature : m_features) {
            if (auto esp_feature = std::dynamic_pointer_cast<EspFeature>(feature)) {
                m_settings_manager->register_settings(&esp_feature->get_settings());
            }
            else if (auto aimbot_feature = std::dynamic_pointer_cast<AimbotFeature>(feature)) {
                m_settings_manager->register_settings(&aimbot_feature->get_settings());
            }
        }
        
        for (auto& feature : m_features) {
            if (feature && m_main_menu) {
                m_main_menu->add_feature(feature);
            }
        }
        
        for (auto& feature : m_features) {
            if (feature && feature->is_initialized()) {
                feature->set_enabled(true);
            }
        }
                
        return true;
    } catch (const std::exception& e) {
        logger::error("Failed to initialize features: " + std::string(e.what()));
        return false;
    }
}

bool Engine::should_update_system(std::chrono::high_resolution_clock::time_point& last_update, double interval_ms) {
    if (m_current_frame_updates >= MAX_UPDATES_PER_FRAME) {
        return false;
    }
    
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - last_update);
    double elapsed_ms = duration.count() / 1000.0;
    
    if (elapsed_ms >= interval_ms) {
        last_update = now;
        m_current_frame_updates++;
        return true;
    }
    
    return false;
}

void Engine::update_caches() {
    if (!should_update_system(m_last_cache_update, CACHE_UPDATE_INTERVAL_MS)) {
        return;
    }
    
    if (m_entity_cache) {
        m_entity_cache->update();
    }
    
    if (m_world_cache) {
        m_world_cache->update();
    }
}

void Engine::update_view() {
    if (!m_access_manager) {
        return;
    }

    m_view_matrix = m_access_manager->read<Matrix4x4>(
        m_entity_cache->get_client_dll_base() + cs2_dumper::offsets::client_dll::dwViewMatrix
    );

    if (should_update_system(m_last_frame_update, FRAME_UPDATE_INTERVAL_MS)) {
        m_entity_cache->update_frame();
    }
}

void Engine::update_physics() {
    if (!should_update_system(m_last_physics_update, PHYSICS_UPDATE_INTERVAL_MS)) {
        return;
    }
    
    if (m_traceline_manager && m_world_cache) {
        if (m_world_cache->triangles_updated()) {
            m_traceline_manager->rebuild_spatial_optimization();
            m_world_cache->mark_triangles_clean();
        }
    }
}

void Engine::update_features() {
    if (!should_update_system(m_last_features_update, FEATURES_UPDATE_INTERVAL_MS)) {
        return;
    }
    
    for (auto& feature : m_features) {
        if (feature && feature->is_initialized() && feature->is_feature_enabled()) {
            try {
                feature->update();
            } catch (const std::exception& e) {
                logger::error("Feature update failed for " + feature->get_name() + ": " + e.what());
            }
        }
    }
}

void Engine::render_features() {
    for (auto& feature : m_features) {
        if (feature && feature->is_initialized() && feature->is_feature_enabled()) {
            try {
                feature->render();
            } catch (const std::exception& e) {
                logger::error("Feature render failed for " + feature->get_name() + ": " + e.what());
            }
        }
    }
}

void Engine::process_input() {
    static bool insert_key_was_pressed = false;
    bool insert_key_pressed = GetAsyncKeyState(VK_INSERT) & 0x8000;
    
    if (insert_key_pressed && !insert_key_was_pressed) {
        settings::g_show_menu = !settings::g_show_menu;
        logger::info("Menu toggled: " + std::string(settings::g_show_menu ? "ON" : "OFF"));
    }
    insert_key_was_pressed = insert_key_pressed;
    
    for (auto& feature : m_features) {
        if (feature && feature->is_initialized() && feature->is_feature_enabled()) {
            try {
                feature->process_input();
            } catch (const std::exception& e) {
                logger::error("Feature input processing failed for " + feature->get_name() + ": " + e.what());
            }
        }
    }
}

void Engine::add_feature(FeaturePtr feature) {
    if (feature) {
        m_features.push_back(feature);
    } else {
        logger::error("Cannot add null feature");
    }
}

bool Engine::world_to_screen(const Vector3& world_pos, Vector2& screen_pos) const {
    if (!m_renderer) {
        return false;
    }
    
    Vector2 screen_size = m_renderer->get_screen_size();
    
    Vector4 clip_coords = {
        world_pos.x * m_view_matrix.m[0][0] + world_pos.y * m_view_matrix.m[0][1] + world_pos.z * m_view_matrix.m[0][2] + m_view_matrix.m[0][3],
        world_pos.x * m_view_matrix.m[1][0] + world_pos.y * m_view_matrix.m[1][1] + world_pos.z * m_view_matrix.m[1][2] + m_view_matrix.m[1][3],
        world_pos.x * m_view_matrix.m[2][0] + world_pos.y * m_view_matrix.m[2][1] + world_pos.z * m_view_matrix.m[2][2] + m_view_matrix.m[2][3],
        world_pos.x * m_view_matrix.m[3][0] + world_pos.y * m_view_matrix.m[3][1] + world_pos.z * m_view_matrix.m[3][2] + m_view_matrix.m[3][3]
    };
    
    if (clip_coords.w < 0.001f) {
        return false;
    }
    
    Vector3 ndc = {
        clip_coords.x / clip_coords.w,
        clip_coords.y / clip_coords.w,
        clip_coords.z / clip_coords.w
    };
    
    if (ndc.x < -1.0f || ndc.x > 1.0f || ndc.y < -1.0f || ndc.y > 1.0f) {
        return false;
    }
    
    screen_pos.x = (ndc.x + 1.0f) * 0.5f * screen_size.x;
    screen_pos.y = (1.0f - ndc.y) * 0.5f * screen_size.y;
    
    return true;
}