#include <features/aimbot/settings/aimbot_settings.hpp>
#include <imgui.h>
#include <vector>

void AimbotSettings::render_imgui() {
    ImGui::Checkbox("Enable Aimbot", &enabled);
    
    if (enabled) {
        ImGui::Separator();
        
        ImGui::Checkbox("Use Aim Key", &aim_key_enabled);
        if (aim_key_enabled) {
            ImGui::SameLine();
            if (ImGui::Button("Set Key")) {

            }
            ImGui::Text("Current Key: 0x%02X", aim_key);
        }
        
        ImGui::SliderFloat("FOV", &fov, 10.0f, 180.0f);
        ImGui::SliderFloat("Smooth", &smooth, 0.1f, 10.0f);
        ImGui::SliderFloat("Max Distance", &max_distance, 50.0f, 1000.0f);
        
        ImGui::Separator();
        ImGui::Text("Target Selection:");
        ImGui::Checkbox("Target Head", &target_head);
        ImGui::Checkbox("Target Chest", &target_chest);
        ImGui::Checkbox("Visible Only", &visible_only);
        ImGui::Checkbox("Team Check", &team_check);
        
        ImGui::Separator();
        ImGui::Text("Visual:");
        ImGui::Checkbox("Show FOV Circle", &show_fov_circle);
        if (show_fov_circle) {
            ImGui::ColorEdit4("FOV Circle Color", (float*)&fov_circle_color);
        }
    }
}

void AimbotSettings::to_json(nlohmann::json& j) const {
    j["enabled"] = enabled;
    j["aim_key_enabled"] = aim_key_enabled;
    j["aim_key"] = aim_key;
    j["fov"] = fov;
    j["smooth"] = smooth;
    j["max_distance"] = max_distance;
    j["target_head"] = target_head;
    j["target_chest"] = target_chest;
    j["visible_only"] = visible_only;
    j["team_check"] = team_check;
    j["show_fov_circle"] = show_fov_circle;
    std::vector<float> fov_color_vec = {fov_circle_color.Value.x, fov_circle_color.Value.y, fov_circle_color.Value.z, fov_circle_color.Value.w};
    j["fov_circle_color"] = fov_color_vec;
}

void AimbotSettings::from_json(const nlohmann::json& j) {
    if (j.contains("enabled")) enabled = j["enabled"];
    if (j.contains("aim_key_enabled")) aim_key_enabled = j["aim_key_enabled"];
    if (j.contains("aim_key")) aim_key = j["aim_key"];
    if (j.contains("fov")) fov = j["fov"];
    if (j.contains("smooth")) smooth = j["smooth"];
    if (j.contains("max_distance")) max_distance = j["max_distance"];
    if (j.contains("target_head")) target_head = j["target_head"];
    if (j.contains("target_chest")) target_chest = j["target_chest"];
    if (j.contains("visible_only")) visible_only = j["visible_only"];
    if (j.contains("team_check")) team_check = j["team_check"];
    if (j.contains("show_fov_circle")) show_fov_circle = j["show_fov_circle"];
    
    if (j.contains("fov_circle_color") && j["fov_circle_color"].is_array() && j["fov_circle_color"].size() == 4) {
        fov_circle_color.Value.x = static_cast<float>(j["fov_circle_color"][0]);
        fov_circle_color.Value.y = static_cast<float>(j["fov_circle_color"][1]);
        fov_circle_color.Value.z = static_cast<float>(j["fov_circle_color"][2]);
        fov_circle_color.Value.w = static_cast<float>(j["fov_circle_color"][3]);
    }
} 