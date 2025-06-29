#pragma once
#include <features/settings/base_settings.hpp>
#include <imgui.h>

class EspSettings : public BaseSettings {
public:
    EspSettings() : BaseSettings("ESP") {}
    
    void render_imgui() override;
    void to_json(nlohmann::json& j) const override;
    void from_json(const nlohmann::json& j) override;
    
    bool enabled = false;
    bool show_boxes = true;
    bool show_names = true;
    bool show_health = true;
    bool show_distance = true;
    float box_thickness = 2.0f;
    float max_distance = 500.0f;
    bool enemy_only = true;

    ImColor box_color = ImColor(1.0f, 0.0f, 0.0f, 1.0f);
    ImColor name_color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
    ImColor health_color = ImColor(0.0f, 1.0f, 0.0f, 1.0f);
}; 