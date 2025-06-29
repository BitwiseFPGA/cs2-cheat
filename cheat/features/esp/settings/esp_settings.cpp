#include <features/esp/settings/esp_settings.hpp>
#include <imgui.h>
#include <vector>

void EspSettings::render_imgui() {
    ImGui::Checkbox("Enable ESP", &enabled);
    
    if (enabled) {
        ImGui::Separator();
        
        ImGui::Checkbox("Show Boxes", &show_boxes);
        ImGui::Checkbox("Show Names", &show_names);
        ImGui::Checkbox("Show Health", &show_health);
        ImGui::Checkbox("Show Distance", &show_distance);
        ImGui::Checkbox("Enemy Only", &enemy_only);
        
        ImGui::SliderFloat("Box Thickness", &box_thickness, 1.0f, 5.0f);
        ImGui::SliderFloat("Max Distance", &max_distance, 50.0f, 1000.0f);
        
        ImGui::Separator();
        ImGui::Text("Colors:");
        ImGui::ColorEdit4("Box Color", (float*)&box_color);
        ImGui::ColorEdit4("Name Color", (float*)&name_color);
        ImGui::ColorEdit4("Health Color", (float*)&health_color);
    }
}

void EspSettings::to_json(nlohmann::json& j) const {
    j["enabled"] = enabled;
    j["show_boxes"] = show_boxes;
    j["show_names"] = show_names;
    j["show_health"] = show_health;
    j["show_distance"] = show_distance;
    j["box_thickness"] = box_thickness;
    j["max_distance"] = max_distance;
    j["enemy_only"] = enemy_only;
    j["box_color"] = box_color;
    j["name_color"] = name_color;
    j["health_color"] = health_color;
}

void EspSettings::from_json(const nlohmann::json& j) {
    if (j.contains("enabled")) enabled = j["enabled"];
    if (j.contains("show_boxes")) show_boxes = j["show_boxes"];
    if (j.contains("show_names")) show_names = j["show_names"];
    if (j.contains("show_health")) show_health = j["show_health"];
    if (j.contains("show_distance")) show_distance = j["show_distance"];
    if (j.contains("box_thickness")) box_thickness = j["box_thickness"];
    if (j.contains("max_distance")) max_distance = j["max_distance"];
    if (j.contains("enemy_only")) enemy_only = j["enemy_only"];
    
    if (j.contains("box_color") && j["box_color"].is_array() && j["box_color"].size() == 4) {
        box_color = j["box_color"];
    }
    if (j.contains("name_color") && j["name_color"].is_array() && j["name_color"].size() == 4) {
        name_color = j["name_color"];
    }
    if (j.contains("health_color") && j["health_color"].is_array() && j["health_color"].size() == 4) {
        health_color = j["health_color"];
    }
} 