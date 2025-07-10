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
        
        // Burst Settings
        ImGui::Text("Burst Settings:");
        ImGui::SliderInt("Burst Shots", &burst_shots, 1, 10, "Shots: %d");
        ImGui::SliderInt("Burst Cooldown", &burst_cooldown_ms, 100, 5000, "Cooldown: %d ms");
        
        ImGui::Separator();
        
        // Reaction Time Settings
        ImGui::Text("Reaction Time Settings:");
        ImGui::SliderInt("Min Reaction Time", &reaction_time_min_ms, 10, 500, "Min: %d ms");
        ImGui::SliderInt("Max Reaction Time", &reaction_time_max_ms, 10, 500, "Max: %d ms");
        
        // Ensure min <= max
        if (reaction_time_min_ms > reaction_time_max_ms) {
            reaction_time_max_ms = reaction_time_min_ms;
        }
    }
}

void TriggerbotSettings::to_json(nlohmann::json& j) const {
    // Main settings
    j["enabled"] = enabled;
    j["trigger_key_enabled"] = trigger_key_enabled;
    j["trigger_key"] = static_cast<int>(trigger_key);
    
    // Burst settings
    j["burst_shots"] = burst_shots;
    j["burst_cooldown_ms"] = burst_cooldown_ms;
    
    // Reaction time settings
    j["reaction_time_min_ms"] = reaction_time_min_ms;
    j["reaction_time_max_ms"] = reaction_time_max_ms;
}

void TriggerbotSettings::from_json(const nlohmann::json& j) {
    // Main settings
    if (j.contains("enabled")) enabled = j["enabled"];
    if (j.contains("trigger_key_enabled")) trigger_key_enabled = j["trigger_key_enabled"];
    if (j.contains("trigger_key")) trigger_key = static_cast<InputKey>(j["trigger_key"].get<int>());
    
    // Burst settings
    if (j.contains("burst_shots")) burst_shots = j["burst_shots"];
    if (j.contains("burst_cooldown_ms")) burst_cooldown_ms = j["burst_cooldown_ms"];
    
    // Reaction time settings
    if (j.contains("reaction_time_min_ms")) reaction_time_min_ms = j["reaction_time_min_ms"];
    if (j.contains("reaction_time_max_ms")) reaction_time_max_ms = j["reaction_time_max_ms"];
} 