#include <menu/main_menu.hpp>
#include <logger/logger.hpp>
#include <imgui.h>
#include <renderer/renderer.hpp>
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
    
    if (ImGui::Begin("CS2 Cheat - Main Menu", &settings::g_show_menu, ImGuiWindowFlags_NoCollapse)) {
        
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
    const ImVec4 COL_BG_PRIMARY = ImVec4(0.086f, 0.086f, 0.086f, 1.00f);      // Primary background #161616
    const ImVec4 COL_BG_SECONDARY = ImVec4(0.102f, 0.102f, 0.102f, 1.00f);    // Secondary background #1A1A1A
    const ImVec4 COL_BG_TERTIARY = ImVec4(0.125f, 0.125f, 0.125f, 1.00f);     // Tertiary background #202020
    const ImVec4 COL_BORDER = ImVec4(0.176f, 0.176f, 0.176f, 1.00f);          // Border color #2D2D2D
    const ImVec4 COL_BORDER_LIGHT = ImVec4(0.220f, 0.220f, 0.220f, 1.00f);    // Light border #383838
    
    const ImVec4 COL_TEXT_PRIMARY = ImVec4(0.918f, 0.918f, 0.918f, 1.00f);    // Primary text #EAEAEA
    const ImVec4 COL_TEXT_SECONDARY = ImVec4(0.729f, 0.729f, 0.729f, 1.00f);  // Secondary text #BABABA
    const ImVec4 COL_TEXT_DISABLED = ImVec4(0.537f, 0.537f, 0.537f, 1.00f);   // Disabled text #898989
    
    const ImVec4 COL_ACCENT_BLUE = ImVec4(0.267f, 0.667f, 1.000f, 1.00f);     // Cursor blue #44AAFF
    const ImVec4 COL_ACCENT_BLUE_HOVER = ImVec4(0.333f, 0.733f, 1.000f, 1.00f); // Hover blue #55BBFF
    const ImVec4 COL_ACCENT_BLUE_ACTIVE = ImVec4(0.200f, 0.600f, 0.933f, 1.00f); // Active blue #3399EE
    
    const ImVec4 COL_HOVER = ImVec4(0.157f, 0.157f, 0.157f, 1.00f);           // Hover state #282828
    const ImVec4 COL_ACTIVE = ImVec4(0.196f, 0.196f, 0.196f, 1.00f);          // Active state #323232
    
    const ImVec4 COL_SUCCESS = ImVec4(0.133f, 0.733f, 0.467f, 1.00f);         // Success green #22BB77
    const ImVec4 COL_WARNING = ImVec4(1.000f, 0.733f, 0.133f, 1.00f);         // Warning orange #FFBB22
    const ImVec4 COL_ERROR = ImVec4(0.933f, 0.267f, 0.267f, 1.00f);           // Error red #EE4444

    ImVec4* colors = ImGui::GetStyle().Colors;
    auto& style = ImGui::GetStyle();
    
    // Text colors
    colors[ImGuiCol_Text] = COL_TEXT_PRIMARY;
    colors[ImGuiCol_TextDisabled] = COL_TEXT_DISABLED;
    colors[ImGuiCol_TextSelectedBg] = ImVec4(COL_ACCENT_BLUE.x, COL_ACCENT_BLUE.y, COL_ACCENT_BLUE.z, 0.35f);
    
    // Background colors
    colors[ImGuiCol_WindowBg] = COL_BG_PRIMARY;
    colors[ImGuiCol_ChildBg] = COL_BG_SECONDARY;
    colors[ImGuiCol_PopupBg] = COL_BG_SECONDARY;
    colors[ImGuiCol_MenuBarBg] = COL_BG_SECONDARY;
    
    // Border colors
    colors[ImGuiCol_Border] = COL_BORDER;
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    
    // Frame colors (inputs, buttons, etc.)
    colors[ImGuiCol_FrameBg] = COL_BG_TERTIARY;
    colors[ImGuiCol_FrameBgHovered] = COL_HOVER;
    colors[ImGuiCol_FrameBgActive] = COL_ACTIVE;
    
    // Title bar colors
    colors[ImGuiCol_TitleBg] = COL_BG_SECONDARY;
    colors[ImGuiCol_TitleBgActive] = COL_BG_SECONDARY;
    colors[ImGuiCol_TitleBgCollapsed] = COL_BG_SECONDARY;
    
    // Scrollbar colors
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    
    // Interactive element colors
    colors[ImGuiCol_CheckMark] = COL_ACCENT_BLUE;
    colors[ImGuiCol_SliderGrab] = COL_ACCENT_BLUE;
    colors[ImGuiCol_SliderGrabActive] = COL_ACCENT_BLUE_ACTIVE;
    
    // Button colors
    colors[ImGuiCol_Button] = COL_BG_TERTIARY;
    colors[ImGuiCol_ButtonHovered] = COL_HOVER;
    colors[ImGuiCol_ButtonActive] = COL_ACTIVE;
    
    // Header colors (collapsing headers, etc.)
    colors[ImGuiCol_Header] = COL_BG_TERTIARY;
    colors[ImGuiCol_HeaderHovered] = COL_HOVER;
    colors[ImGuiCol_HeaderActive] = COL_ACTIVE;
    
    // Separator colors
    colors[ImGuiCol_Separator] = COL_BORDER;
    colors[ImGuiCol_SeparatorHovered] = COL_BORDER_LIGHT;
    colors[ImGuiCol_SeparatorActive] = COL_ACCENT_BLUE;
    
    // Resize grip colors
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.37f, 0.37f, 0.37f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.44f, 0.44f, 0.44f, 0.84f);
    
    // Tab colors
    colors[ImGuiCol_Tab] = COL_BG_SECONDARY;
    colors[ImGuiCol_TabHovered] = COL_HOVER;
    colors[ImGuiCol_TabActive] = COL_BG_TERTIARY;
    colors[ImGuiCol_TabUnfocused] = COL_BG_SECONDARY;
    colors[ImGuiCol_TabUnfocusedActive] = COL_BG_TERTIARY;
    
    // Plot colors
    colors[ImGuiCol_PlotLines] = COL_ACCENT_BLUE;
    colors[ImGuiCol_PlotLinesHovered] = COL_ACCENT_BLUE_HOVER;
    colors[ImGuiCol_PlotHistogram] = COL_ACCENT_BLUE;
    colors[ImGuiCol_PlotHistogramHovered] = COL_ACCENT_BLUE_HOVER;
    
    // Table colors
    colors[ImGuiCol_TableHeaderBg] = COL_BG_SECONDARY;
    colors[ImGuiCol_TableBorderStrong] = COL_BORDER;
    colors[ImGuiCol_TableBorderLight] = COL_BORDER;
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);

    // Cursor-style rounding and spacing
    style.WindowRounding = 12.0f;
    style.ChildRounding = 12.0f;
    style.FrameRounding = 12.0f;
    style.PopupRounding = 12.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 12.0f;
    style.TabRounding = 12.0f;
    
    // Cursor-style spacing and padding
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.FramePadding = ImVec2(8.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 12.0f;
    
    // Cursor-style borders
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.TabBorderSize = 0.0f;

    // Other Cursor-style settings
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_None;
    style.ColorButtonPosition = ImGuiDir_Left;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.5f);
    style.SeparatorTextBorderSize = 1.0f;

    auto& io = ImGui::GetIO();

    io.Fonts->Clear();  // Clear old fonts
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    
}
