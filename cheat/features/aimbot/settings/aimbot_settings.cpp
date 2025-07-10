#include <features/aimbot/settings/aimbot_settings.hpp>
#include <input/adapter/base_input.h>

#include <imgui.h>
#include <vector>
#include <unordered_map>
#include <string>

void AimbotSettings::render_imgui() {
    ImGui::Checkbox("Enable Aimbot", &enabled);
    
    if (!enabled) {
        return;
    }
    
    ImGui::Separator();
    
    // Main Settings Section
    ImGui::Text("Main Settings:");
    ImGui::Checkbox("Use Aim Key", &aim_key_enabled);
    
    if (aim_key_enabled) {
        static std::unordered_map<InputKey, std::string> key_names = {
            {InputKey::MouseLeft, "Mouse Left"},
            {InputKey::MouseRight, "Mouse Right"},
            {InputKey::MouseMiddle, "Mouse Middle"},
            {InputKey::MouseX1, "Mouse X1"},
            {InputKey::MouseX2, "Mouse X2"},
            {InputKey::Shift, "Shift"},
            {InputKey::Ctrl, "Ctrl"},
            {InputKey::Alt, "Alt"},
            {InputKey::Space, "Space"},
            {InputKey::F1, "F1"}, {InputKey::F2, "F2"}, {InputKey::F3, "F3"},
            {InputKey::F4, "F4"}, {InputKey::F5, "F5"}, {InputKey::F6, "F6"}
        };
        
        std::string current_key_name = "Unknown";
        if (key_names.find(aim_key) != key_names.end()) {
            current_key_name = key_names[aim_key];
        }
        
        if (ImGui::BeginCombo("Aim Key", current_key_name.c_str())) {
            for (const auto& [key, name] : key_names) {
                bool is_selected = (aim_key == key);
                if (ImGui::Selectable(name.c_str(), is_selected)) {
                    aim_key = key;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }
    
    ImGui::Separator();
    
    // Target Selection Section
    ImGui::Text("Target Selection:");
    ImGui::Checkbox("Multi-Bone Targeting", &multi_bone_enabled);
    
    if (multi_bone_enabled) {
        ImGui::Text("Select Target Bones:");
        ImGui::Checkbox("Head", &target_head);
        ImGui::Checkbox("Neck", &target_neck);
        ImGui::Checkbox("Chest", &target_chest);
        ImGui::Checkbox("Spine", &target_spine);
        ImGui::Checkbox("Hip", &target_hip);
    } else {
        ImGui::Text("Primary Target Bone:");
        static const char* bone_names[] = { "Head", "Neck", "Chest", "Spine", "Hip" };
        int current_bone = static_cast<int>(target_bone);
        if (ImGui::Combo("Target Bone", &current_bone, bone_names, IM_ARRAYSIZE(bone_names))) {
            target_bone = static_cast<AimbotBone>(current_bone);
        }
    }
    
    ImGui::Separator();
    
    // Filtering Section
    ImGui::Text("Filtering:");
    ImGui::Checkbox("Visible Only", &visible_only);
    ImGui::Checkbox("Team Check", &team_check);
    ImGui::SliderFloat("FOV", &fov, 10.0f, 180.0f, "%.1f°");
    ImGui::SliderFloat("Max Distance", &max_distance, 50.0f, 1000.0f, "%.0fm");
    
    ImGui::Separator();
    
    // Smoothing Section
    ImGui::Text("Smoothing:");
    ImGui::SliderFloat("Smooth", &smooth, 1.0f, 10.0f, "%.1f");
    ImGui::Checkbox("Recoil Control System", &rcs_enabled);
    
    ImGui::Separator();
    
    // Visual Feedback Section
    ImGui::Text("Visual Feedback:");
    ImGui::Checkbox("Show FOV Circle", &show_fov_circle);
    if (show_fov_circle) {
        ImGui::ColorEdit4("FOV Circle Color", (float*)&fov_circle_color);
    }
    ImGui::Checkbox("Show Aim Line", &show_aim_line);
}

void AimbotSettings::to_json(nlohmann::json& j) const {
    // Main settings
    j["enabled"] = enabled;
    j["aim_key_enabled"] = aim_key_enabled;
    j["aim_key"] = static_cast<int>(aim_key);
    
    // Target selection
    j["target_bone"] = static_cast<int>(target_bone);
    j["multi_bone_enabled"] = multi_bone_enabled;
    j["target_head"] = target_head;
    j["target_neck"] = target_neck;
    j["target_chest"] = target_chest;
    j["target_spine"] = target_spine;
    j["target_hip"] = target_hip;
    
    // Filtering
    j["visible_only"] = visible_only;
    j["team_check"] = team_check;
    j["fov"] = fov;
    j["max_distance"] = max_distance;
    
    // Smoothing
    j["smooth"] = smooth;
    j["rcs_enabled"] = rcs_enabled;
    
    // Visual feedback
    j["show_fov_circle"] = show_fov_circle;
    std::vector<float> fov_color_vec = {fov_circle_color.Value.x, fov_circle_color.Value.y, fov_circle_color.Value.z, fov_circle_color.Value.w};
    j["fov_circle_color"] = fov_color_vec;
    j["show_aim_line"] = show_aim_line;
}

void AimbotSettings::from_json(const nlohmann::json& j) {
    // Main settings
    if (j.contains("enabled")) enabled = j["enabled"];
    if (j.contains("aim_key_enabled")) aim_key_enabled = j["aim_key_enabled"];
    if (j.contains("aim_key")) aim_key = static_cast<InputKey>(j["aim_key"].get<int>());
    
    // Target selection
    if (j.contains("target_bone")) target_bone = static_cast<AimbotBone>(j["target_bone"].get<int>());
    if (j.contains("multi_bone_enabled")) multi_bone_enabled = j["multi_bone_enabled"];
    if (j.contains("target_head")) target_head = j["target_head"];
    if (j.contains("target_neck")) target_neck = j["target_neck"];
    if (j.contains("target_chest")) target_chest = j["target_chest"];
    if (j.contains("target_spine")) target_spine = j["target_spine"];
    if (j.contains("target_hip")) target_hip = j["target_hip"];
    
    // Filtering
    if (j.contains("visible_only")) visible_only = j["visible_only"];
    if (j.contains("team_check")) team_check = j["team_check"];
    if (j.contains("fov")) fov = j["fov"];
    if (j.contains("max_distance")) max_distance = j["max_distance"];
    
    // Smoothing
    if (j.contains("smooth")) smooth = j["smooth"];
    if (j.contains("rcs_enabled")) rcs_enabled = j["rcs_enabled"];
    
    // Visual feedback
    if (j.contains("show_fov_circle")) show_fov_circle = j["show_fov_circle"];
    if (j.contains("fov_circle_color") && j["fov_circle_color"].is_array() && j["fov_circle_color"].size() == 4) {
        fov_circle_color.Value.x = static_cast<float>(j["fov_circle_color"][0]);
        fov_circle_color.Value.y = static_cast<float>(j["fov_circle_color"][1]);
        fov_circle_color.Value.z = static_cast<float>(j["fov_circle_color"][2]);
        fov_circle_color.Value.w = static_cast<float>(j["fov_circle_color"][3]);
    }
    if (j.contains("show_aim_line")) show_aim_line = j["show_aim_line"];
} 