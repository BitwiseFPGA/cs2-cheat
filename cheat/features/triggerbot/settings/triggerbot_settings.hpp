#pragma once
#include <features/settings/base_settings.hpp>
#include <input/adapter/base_input.h>
#include <imgui.h>

class TriggerbotSettings : public BaseSettings {
public:
    TriggerbotSettings() : BaseSettings("Triggerbot") {}
    
    void render_imgui() override;
    void to_json(nlohmann::json& j) const override;
    void from_json(const nlohmann::json& j) override;
    
    // Main settings
    bool enabled = true;
    bool trigger_key_enabled = false;
    InputKey trigger_key = InputKey::Alt;
    
    // Trigger settings
    float trigger_delay_min = 50.0f;  // Minimum delay in ms
    float trigger_delay_max = 100.0f; // Maximum delay in ms
    float crosshair_tolerance = 5.0f; // Pixels around crosshair center
    bool auto_scope_only = false;     // Only trigger when scoped
    
    // Filtering
    bool visible_only = true;
    bool team_check = true;
    bool head_only = false;           // Only trigger on head shots
    float max_distance = 300.0f;
    
    // Weapon filtering
    bool rifle_enabled = true;
    bool pistol_enabled = true;
    bool sniper_enabled = true;
    bool smg_enabled = true;
    
    // Safety
    bool flash_check = true;          // Don't trigger when flashed
    bool smoke_check = true;          // Don't trigger through smoke
    
    // Visual feedback
    bool show_crosshair_circle = true;
    ImColor crosshair_color = ImColor(1.0f, 0.0f, 0.0f, 0.8f);
    bool show_trigger_indicator = true;
    bool show_target_info = true;
}; 