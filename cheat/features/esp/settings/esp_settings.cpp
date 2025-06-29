#include <features/esp/settings/esp_settings.hpp>
#include <imgui.h>
#include <vector>

void EspSettings::render_imgui() {
    ImGui::Checkbox("Enable ESP", &enabled);
    
    if (enabled) {
        ImGui::Separator();
        
        // Basic options
        ImGui::Checkbox("Show Boxes", &show_boxes);
        ImGui::Checkbox("Show Names", &show_names);
        ImGui::Checkbox("Show Health", &show_health);
        ImGui::Checkbox("Enemy Only", &enemy_only);
        ImGui::Checkbox("Player Visibility Check", &player_visibility_check);
        
        
        ImGui::Separator();
        ImGui::Text("Box Type:");
        const char* box_type_items[] = { "None", "2D", "2D Corner" };
        int current_box_type = static_cast<int>(box_type);
        if (ImGui::Combo("##BoxType", &current_box_type, box_type_items, IM_ARRAYSIZE(box_type_items))) {
            box_type = static_cast<BOX_TYPE>(current_box_type);
        }
        
        ImGui::Separator();
        ImGui::Text("Player Info:");
        ImGui::CheckboxFlags("Name", &player_info, PLAYER_INFO_NAME);
        ImGui::CheckboxFlags("Distance", &player_info, PLAYER_INFO_DISTANCE);
        ImGui::CheckboxFlags("Weapon", &player_info, PLAYER_INFO_WEAPON);
        ImGui::CheckboxFlags("Skeleton", &player_info, PLAYER_INFO_SKELETON);
        ImGui::CheckboxFlags("Health Bar", &player_info, PLAYER_INFO_HEALTHBAR);
        ImGui::CheckboxFlags("Armor Bar", &player_info, PLAYER_INFO_ARMORBAR);
        ImGui::CheckboxFlags("C4 Carrier", &player_info, PLAYER_INFO_C4_CARRIER);
        ImGui::CheckboxFlags("Defuser", &player_info, PLAYER_INFO_DEFUSER);
        ImGui::CheckboxFlags("Helmet", &player_info, PLAYER_INFO_HELMET);
        
        ImGui::Separator();
        ImGui::Text("Colors:");
        ImGui::ColorEdit4("Box Color", (float*)&box_color);
        ImGui::ColorEdit4("Name Color", (float*)&name_color);
        ImGui::ColorEdit4("Health Color", (float*)&health_color);
        ImGui::ColorEdit4("Player Visible Color", (float*)&player_color_visible);
        ImGui::ColorEdit4("Player Invisible Color", (float*)&player_color_invisible);
    }
}

void EspSettings::to_json(nlohmann::json& j) const {
    j["enabled"] = enabled;
    j["show_boxes"] = show_boxes;
    j["show_names"] = show_names;
    j["show_health"] = show_health;
    j["enemy_only"] = enemy_only;
    j["box_type"] = static_cast<int>(box_type);
    j["player_info"] = player_info;
    j["player_visibility_check"] = player_visibility_check;
    j["box_color"] = box_color;
    j["name_color"] = name_color;
    j["health_color"] = health_color;
    j["player_color_visible"] = player_color_visible;
    j["player_color_invisible"] = player_color_invisible;
}

void EspSettings::from_json(const nlohmann::json& j) {
    if (j.contains("enabled")) enabled = j["enabled"];
    if (j.contains("show_boxes")) show_boxes = j["show_boxes"];
    if (j.contains("show_names")) show_names = j["show_names"];
    if (j.contains("show_health")) show_health = j["show_health"];
    if (j.contains("enemy_only")) enemy_only = j["enemy_only"];
    if (j.contains("box_type")) box_type = static_cast<BOX_TYPE>(j["box_type"]);
    if (j.contains("player_info")) player_info = j["player_info"];
    if (j.contains("player_visibility_check")) player_visibility_check = j["player_visibility_check"];
    
    if (j.contains("box_color") && j["box_color"].is_array() && j["box_color"].size() == 4) {
        box_color = j["box_color"];
    }
    if (j.contains("name_color") && j["name_color"].is_array() && j["name_color"].size() == 4) {
        name_color = j["name_color"];
    }
    if (j.contains("health_color") && j["health_color"].is_array() && j["health_color"].size() == 4) {
        health_color = j["health_color"];
    }
    if (j.contains("player_color_visible") && j["player_color_visible"].is_array() && j["player_color_visible"].size() == 4) {
        player_color_visible = j["player_color_visible"];
    }
    if (j.contains("player_color_invisible") && j["player_color_invisible"].is_array() && j["player_color_invisible"].size() == 4) {
        player_color_invisible = j["player_color_invisible"];
    }
} 