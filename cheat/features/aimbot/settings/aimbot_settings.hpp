#pragma once
#include <features/settings/base_settings.hpp>
#include <input/adapter/base_input.h>

#include <imgui.h>

enum class AimbotBone {
    HEAD = 0,
    NECK = 1,
    CHEST = 2,
    SPINE = 3,
    HIP = 4
};

class AimbotSettings : public BaseSettings {
public:
    AimbotSettings() : BaseSettings("Aimbot") {}
    
    void render_imgui() override;
    void to_json(nlohmann::json& j) const override;
    void from_json(const nlohmann::json& j) override;
    
    // Main settings
    bool enabled = true;
    bool aim_key_enabled = true;
    InputKey aim_key = InputKey::Alt;
    
    // Target selection
    AimbotBone target_bone = AimbotBone::HEAD;
    bool multi_bone_enabled = false;
    bool target_head = true;
    bool target_neck = false;
    bool target_chest = false;
    bool target_spine = false;
    bool target_hip = false;
    
    // Filtering
    bool visible_only = true;
    bool team_check = true;
    float fov = 90.0f;
    float max_distance = 300.0f;
    
    // Smoothing
    float smooth = 4.0f;
    bool rcs_enabled = true;
    
    // Visual feedback
    bool show_fov_circle = true;
    ImColor fov_circle_color = ImColor(1.0f, 1.0f, 1.0f, 0.5f);
    bool show_aim_line = false;
}; 