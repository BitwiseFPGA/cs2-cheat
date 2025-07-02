#pragma once
#include <features/settings/base_settings.hpp>
#include <input/adapter/base_input.h>
#include <imgui.h>

class AimbotSettings : public BaseSettings {
public:
    AimbotSettings() : BaseSettings("Aimbot") {}
    
    void render_imgui() override;
    void to_json(nlohmann::json& j) const override;
    void from_json(const nlohmann::json& j) override;
    
    bool enabled = false;
    bool aim_key_enabled = true;
    InputKey aim_key = InputKey::MouseLeft;
    float fov = 90.0f;
    float smooth = 1.0f;
    float max_distance = 300.0f;
    bool target_head = true;
    bool target_chest = false;
    bool visible_only = true;
    bool team_check = true;
    bool show_fov_circle = true;
    ImColor fov_circle_color = ImColor(1.0f, 1.0f, 1.0f, 0.5f);
}; 