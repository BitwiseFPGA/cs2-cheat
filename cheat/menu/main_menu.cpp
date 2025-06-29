#include <menu/main_menu.hpp>
#include <logger/logger.hpp>
#include <imgui.h>
#include <engine/renderer/renderer.hpp>
#include <config/runtime/settings.hpp>
#include <features/base_feature.hpp>
#include <features/esp/esp.hpp>
#include <features/aimbot/aimbot.hpp>
#include <config/build/settings.hpp>

MainMenu::MainMenu()
    : m_initialized(false)
    , m_renderer(nullptr)
    , m_settings_manager(nullptr)
{
}

MainMenu::~MainMenu() {
    if (m_initialized) {
        shutdown();
    }
}

bool MainMenu::initialize(Renderer* renderer, SettingsManager* settings_manager) {
    logger::debug("Initializing main menu");
    
    setup_style();
    
    m_renderer = renderer;
    m_settings_manager = settings_manager;
    
    m_initialized = true;
    return true;
}

void MainMenu::shutdown() {
    logger::debug("Shutting down main menu");
    m_features.clear();
    m_initialized = false;
}

void MainMenu::add_feature(std::shared_ptr<BaseFeature> feature) {
    if (feature) {
        m_features.push_back(feature);
        logger::debug("Added feature to menu: " + feature->get_name());
    }
}

void MainMenu::render_feature_settings() {
    if (ImGui::BeginTabBar("FeatureTabs")) {
        
        for (auto& feature : m_features) {
            if (ImGui::BeginTabItem(feature->get_name().c_str())) {
                
                if (auto esp_feature = std::dynamic_pointer_cast<EspFeature>(feature)) {
                    esp_feature->get_settings().render_imgui();
                }
                else if (auto aimbot_feature = std::dynamic_pointer_cast<AimbotFeature>(feature)) {
                    aimbot_feature->get_settings().render_imgui();
                }
                else {
                    ImGui::Text("Feature: %s", feature->get_name().c_str());
                    ImGui::Text("Status: %s", feature->is_initialized() ? "Initialized" : "Not Initialized");
                    
                    bool enabled = feature->is_enabled();
                    if (ImGui::Checkbox("Enabled", &enabled)) {
                        feature->set_enabled(enabled);
                    }
                }
                
                ImGui::EndTabItem();
            }
        }
        
        ImGui::EndTabBar();
    }
}

void MainMenu::render() {
    if (!m_initialized || !settings::g_show_menu) {
        return;
    }
    
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Template Cheat - Main Menu", &settings::g_show_menu, ImGuiWindowFlags_NoCollapse)) {
        
        if (ImGui::BeginTabBar("MainTabs")) {
            
            if (ImGui::BeginTabItem("Main")) {
                ImGui::Text("Hello World!");
                ImGui::Text("Renderer Framerate: %.1f FPS", m_renderer->get_framerate());
                
                if (ImGui::Button("Close Menu")) {
                    settings::g_show_menu = false;
                }
                
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Features")) {
                render_feature_settings();
                ImGui::EndTabItem();
            }
            
            if (ImGui::BeginTabItem("Config")) {
                ImGui::Text("Configuration options will go here");
                
                if (ImGui::Button("Save Config")) {
                    if (m_settings_manager) {
                        if (m_settings_manager->save()) {
                            logger::info("Config saved successfully");
                        } else {
                            logger::error("Failed to save config");
                        }
                    }
                }
                
                ImGui::SameLine();
                if (ImGui::Button("Load Config")) {
                    if (m_settings_manager) {
                        if (m_settings_manager->load()) {
                            logger::info("Config loaded successfully");
                        } else {
                            logger::error("Failed to load config");
                        }
                    }
                }
                
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

void MainMenu::setup_style() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.Colors[ImGuiCol_WindowBg].w = 0.9f;
    style.Colors[ImGuiCol_ChildBg].w = 0.9f;
    
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
}
