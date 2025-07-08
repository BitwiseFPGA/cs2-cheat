#include <features/triggerbot/settings/triggerbot_settings.hpp>
#include <input/adapter/base_input.h>

#include <imgui.h>
#include <vector>
#include <unordered_map>
#include <string>

void TriggerbotSettings::render_imgui() {
    ImGui::Checkbox("Enable Triggerbot", &enabled);
    
    if (enabled) {
        ImGui::Separator();
        
        // Main Settings
        ImGui::Text("Main Settings:");
        ImGui::Checkbox("Use Trigger Key", &trigger_key_enabled);
        if (trigger_key_enabled) {
            // Create key name mapping
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
            if (key_names.find(trigger_key) != key_names.end()) {
                current_key_name = key_names[trigger_key];
            }
            
            if (ImGui::BeginCombo("Trigger Key", current_key_name.c_str())) {
                for (const auto& [key, name] : key_names) {
                    bool is_selected = (trigger_key == key);
                    if (ImGui::Selectable(name.c_str(), is_selected)) {
                        trigger_key = key;
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }
        
        ImGui::Separator();
        
        // Trigger Settings
        ImGui::Text("Trigger Settings:");
        ImGui::SliderFloat("Min Delay (ms)", &trigger_delay_min, 0.0f, 500.0f, "%.0f");
        ImGui::SliderFloat("Max Delay (ms)", &trigger_delay_max, 0.0f, 500.0f, "%.0f");
        
        // Ensure max >= min
        if (trigger_delay_max < trigger_delay_min) {
            trigger_delay_max = trigger_delay_min;
        }
        
        ImGui::SliderFloat("Crosshair Tolerance", &crosshair_tolerance, 1.0f, 20.0f, "%.1f px");
        ImGui::Checkbox("Auto-Scope Only", &auto_scope_only);
        
        ImGui::Separator();
        
        // Filtering
        ImGui::Text("Filtering:");
        ImGui::Checkbox("Visible Only", &visible_only);
        ImGui::Checkbox("Team Check", &team_check);
        ImGui::Checkbox("Head Only", &head_only);
        ImGui::SliderFloat("Max Distance", &max_distance, 50.0f, 1000.0f, "%.0fm");
        
        ImGui::Separator();
        
        // Weapon Filtering
        ImGui::Text("Weapon Filtering:");
        ImGui::Checkbox("Rifles", &rifle_enabled);
        ImGui::Checkbox("Pistols", &pistol_enabled);
        ImGui::Checkbox("Snipers", &sniper_enabled);
        ImGui::Checkbox("SMGs", &smg_enabled);
        
        ImGui::Separator();
        
        // Safety
        ImGui::Text("Safety:");
        ImGui::Checkbox("Flash Check", &flash_check);
        ImGui::Checkbox("Smoke Check", &smoke_check);
        
        ImGui::Separator();
        
        // Visual Feedback
        ImGui::Text("Visual Feedback:");
        ImGui::Checkbox("Show Crosshair Circle", &show_crosshair_circle);
        if (show_crosshair_circle) {
            ImGui::ColorEdit4("Crosshair Color", (float*)&crosshair_color);
        }
        ImGui::Checkbox("Show Trigger Indicator", &show_trigger_indicator);
        ImGui::Checkbox("Show Target Info", &show_target_info);
    }
}

void TriggerbotSettings::to_json(nlohmann::json& j) const {
    // Main settings
    j["enabled"] = enabled;
    j["trigger_key_enabled"] = trigger_key_enabled;
    j["trigger_key"] = static_cast<int>(trigger_key);
    
    // Trigger settings
    j["trigger_delay_min"] = trigger_delay_min;
    j["trigger_delay_max"] = trigger_delay_max;
    j["crosshair_tolerance"] = crosshair_tolerance;
    j["auto_scope_only"] = auto_scope_only;
    
    // Filtering
    j["visible_only"] = visible_only;
    j["team_check"] = team_check;
    j["head_only"] = head_only;
    j["max_distance"] = max_distance;
    
    // Weapon filtering
    j["rifle_enabled"] = rifle_enabled;
    j["pistol_enabled"] = pistol_enabled;
    j["sniper_enabled"] = sniper_enabled;
    j["smg_enabled"] = smg_enabled;
    
    // Safety
    j["flash_check"] = flash_check;
    j["smoke_check"] = smoke_check;
    
    // Visual feedback
    j["show_crosshair_circle"] = show_crosshair_circle;
    std::vector<float> crosshair_color_vec = {crosshair_color.Value.x, crosshair_color.Value.y, crosshair_color.Value.z, crosshair_color.Value.w};
    j["crosshair_color"] = crosshair_color_vec;
    j["show_trigger_indicator"] = show_trigger_indicator;
    j["show_target_info"] = show_target_info;
}

void TriggerbotSettings::from_json(const nlohmann::json& j) {
    // Main settings
    if (j.contains("enabled")) enabled = j["enabled"];
    if (j.contains("trigger_key_enabled")) trigger_key_enabled = j["trigger_key_enabled"];
    if (j.contains("trigger_key")) trigger_key = static_cast<InputKey>(j["trigger_key"].get<int>());
    
    // Trigger settings
    if (j.contains("trigger_delay_min")) trigger_delay_min = j["trigger_delay_min"];
    if (j.contains("trigger_delay_max")) trigger_delay_max = j["trigger_delay_max"];
    if (j.contains("crosshair_tolerance")) crosshair_tolerance = j["crosshair_tolerance"];
    if (j.contains("auto_scope_only")) auto_scope_only = j["auto_scope_only"];
    
    // Filtering
    if (j.contains("visible_only")) visible_only = j["visible_only"];
    if (j.contains("team_check")) team_check = j["team_check"];
    if (j.contains("head_only")) head_only = j["head_only"];
    if (j.contains("max_distance")) max_distance = j["max_distance"];
    
    // Weapon filtering
    if (j.contains("rifle_enabled")) rifle_enabled = j["rifle_enabled"];
    if (j.contains("pistol_enabled")) pistol_enabled = j["pistol_enabled"];
    if (j.contains("sniper_enabled")) sniper_enabled = j["sniper_enabled"];
    if (j.contains("smg_enabled")) smg_enabled = j["smg_enabled"];
    
    // Safety
    if (j.contains("flash_check")) flash_check = j["flash_check"];
    if (j.contains("smoke_check")) smoke_check = j["smoke_check"];
    
    // Visual feedback
    if (j.contains("show_crosshair_circle")) show_crosshair_circle = j["show_crosshair_circle"];
    if (j.contains("crosshair_color") && j["crosshair_color"].is_array() && j["crosshair_color"].size() == 4) {
        crosshair_color.Value.x = static_cast<float>(j["crosshair_color"][0]);
        crosshair_color.Value.y = static_cast<float>(j["crosshair_color"][1]);
        crosshair_color.Value.z = static_cast<float>(j["crosshair_color"][2]);
        crosshair_color.Value.w = static_cast<float>(j["crosshair_color"][3]);
    }
    if (j.contains("show_trigger_indicator")) show_trigger_indicator = j["show_trigger_indicator"];
    if (j.contains("show_target_info")) show_target_info = j["show_target_info"];
} 