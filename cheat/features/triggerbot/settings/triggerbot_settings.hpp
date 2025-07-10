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
    
    // Burst settings
    int burst_shots = 3;
    int burst_cooldown_ms = 1000;
    
    // Reaction time settings
    int reaction_time_min_ms = 50;
    int reaction_time_max_ms = 150;
    
}; 