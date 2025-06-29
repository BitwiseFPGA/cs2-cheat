#pragma once
#include <features/settings/base_settings.hpp>
#include <imgui.h>

enum class BOX_TYPE {
    BOX_NONE,
    BOX_2D,
    BOX_2D_CORNER
};

enum PLAYER_INFO {
    PLAYER_INFO_NAME = 1 << 0,
    PLAYER_INFO_DISTANCE = 1 << 1,
    PLAYER_INFO_WEAPON = 1 << 2,
    PLAYER_INFO_SKELETON = 1 << 3,
    PLAYER_INFO_HEALTHBAR = 1 << 4,
    PLAYER_INFO_ARMORBAR = 1 << 5,
    PLAYER_INFO_C4_CARRIER = 1 << 6,
    PLAYER_INFO_DEFUSER = 1 << 7,
    PLAYER_INFO_HELMET = 1 << 8
};

class EspSettings : public BaseSettings {
public:
    EspSettings() : BaseSettings("ESP") {}
    
    void render_imgui() override;
    void to_json(nlohmann::json& j) const override;
    void from_json(const nlohmann::json& j) override;
    
    bool enabled = true;
    bool show_boxes = true;
    bool show_names = true;
    bool show_health = true;
    bool enemy_only = true;

    BOX_TYPE box_type = BOX_TYPE::BOX_2D;
    int player_info = PLAYER_INFO_NAME | PLAYER_INFO_WEAPON | PLAYER_INFO_HEALTHBAR | PLAYER_INFO_SKELETON;
    bool player_visibility_check = true;

    ImColor box_color = ImColor(1.0f, 0.0f, 0.0f, 1.0f);
    ImColor name_color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
    ImColor health_color = ImColor(0.0f, 1.0f, 0.0f, 1.0f);
    ImColor player_color_visible = ImColor(0.0f, 1.0f, 0.0f, 1.0f);
    ImColor player_color_invisible = ImColor(1.0f, 0.0f, 0.0f, 1.0f);
}; 