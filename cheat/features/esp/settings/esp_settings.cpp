#include <features/esp/settings/esp_settings.hpp>
#include <imgui.h>
#include <vector>

void EspSettings::render_imgui() {
    ImGui::Checkbox("Enable ESP", &enabled);
    
    if (!enabled) {
        return;
    }
    
    ImGui::Separator();
    
    // Create tab bar
    if (ImGui::BeginTabBar("ESPTabs")) {
        
        // Player ESP Tab
        if (ImGui::BeginTabItem("Players")) {
            ImGui::Checkbox("Enable Player ESP", &player.enabled);
            
            if (player.enabled) {
                ImGui::Separator();
                
                // Player filters
                ImGui::Text("Filters:");
                ImGui::Checkbox("Enemy Only", &player.enemy_only);
                ImGui::Checkbox("Visibility Check", &player.visibility_check);
                ImGui::SliderFloat("Max Distance", &player.max_distance, 10.0f, 1000.0f, "%.0fm");
                
                ImGui::Separator();
                
                // Box settings
                ImGui::Text("Box Type:");
                const char* box_type_items[] = { "None", "2D", "2D Corner" };
                int current_box_type = static_cast<int>(player.box_type);
                if (ImGui::Combo("##PlayerBoxType", &current_box_type, box_type_items, IM_ARRAYSIZE(box_type_items))) {
                    player.box_type = static_cast<BOX_TYPE>(current_box_type);
                }
                
                ImGui::Separator();
                
                // Player info flags
                ImGui::Text("Player Info:");
                ImGui::CheckboxFlags("Name", &player.player_info, PLAYER_INFO_NAME);
                ImGui::CheckboxFlags("Distance", &player.player_info, PLAYER_INFO_DISTANCE);
                ImGui::CheckboxFlags("Weapon", &player.player_info, PLAYER_INFO_WEAPON);
                ImGui::CheckboxFlags("Skeleton", &player.player_info, PLAYER_INFO_SKELETON);
                ImGui::CheckboxFlags("Health Bar", &player.player_info, PLAYER_INFO_HEALTHBAR);
                ImGui::CheckboxFlags("Armor Bar", &player.player_info, PLAYER_INFO_ARMORBAR);
                ImGui::CheckboxFlags("C4 Carrier", &player.player_info, PLAYER_INFO_C4_CARRIER);
                ImGui::CheckboxFlags("Defuser", &player.player_info, PLAYER_INFO_DEFUSER);
                ImGui::CheckboxFlags("Helmet", &player.player_info, PLAYER_INFO_HELMET);
                
                ImGui::Separator();
                
                // Player colors
                ImGui::Text("Colors:");
                ImGui::ColorEdit4("Visible Color", (float*)&player.color_visible);
                ImGui::ColorEdit4("Invisible Color", (float*)&player.color_invisible);
                ImGui::ColorEdit4("Name Color", (float*)&player.name_color);
                ImGui::ColorEdit4("Box Color", (float*)&player.box_color);
            }
            
            ImGui::EndTabItem();
        }
        
        // Entity ESP Tab
        if (ImGui::BeginTabItem("Entities")) {
            ImGui::Checkbox("Enable Entity ESP", &entities.enabled);
            
            if (entities.enabled) {
                ImGui::Separator();
                
                // Entity type filters
                ImGui::Text("Entity Types:");
                ImGui::CheckboxFlags("Weapons", &entities.entity_types, ENTITY_TYPE_WEAPONS);
                ImGui::CheckboxFlags("Grenades", &entities.entity_types, ENTITY_TYPE_GRENADES);
                ImGui::CheckboxFlags("C4", &entities.entity_types, ENTITY_TYPE_C4);
                ImGui::CheckboxFlags("Hostages", &entities.entity_types, ENTITY_TYPE_HOSTAGES);
                ImGui::CheckboxFlags("Chickens", &entities.entity_types, ENTITY_TYPE_CHICKENS);
                ImGui::CheckboxFlags("Other", &entities.entity_types, ENTITY_TYPE_OTHER);
                
                ImGui::Separator();
                
                // Entity info flags
                ImGui::Text("Entity Info:");
                ImGui::CheckboxFlags("Name", &entities.entity_info, ENTITY_INFO_NAME);
                ImGui::CheckboxFlags("Distance", &entities.entity_info, ENTITY_INFO_DISTANCE);
                ImGui::CheckboxFlags("Box", &entities.entity_info, ENTITY_INFO_BOX);
                
                ImGui::Separator();
                
                // Entity settings
                ImGui::SliderFloat("Max Distance", &entities.max_distance, 10.0f, 500.0f, "%.0fm");
                
                ImGui::Text("Box Type:");
                const char* entity_box_type_items[] = { "None", "2D", "2D Corner" };
                int current_entity_box_type = static_cast<int>(entities.box_type);
                if (ImGui::Combo("##EntityBoxType", &current_entity_box_type, entity_box_type_items, IM_ARRAYSIZE(entity_box_type_items))) {
                    entities.box_type = static_cast<BOX_TYPE>(current_entity_box_type);
                }
                
                ImGui::Separator();
                
                // Entity colors
                ImGui::Text("Colors:");
                ImGui::ColorEdit4("Weapon Color", (float*)&entities.weapon_color);
                ImGui::ColorEdit4("Grenade Color", (float*)&entities.grenade_color);
                ImGui::ColorEdit4("C4 Color", (float*)&entities.c4_color);
                ImGui::ColorEdit4("Hostage Color", (float*)&entities.hostage_color);
                ImGui::ColorEdit4("Chicken Color", (float*)&entities.chicken_color);
                ImGui::ColorEdit4("Other Color", (float*)&entities.other_color);
                ImGui::ColorEdit4("Name Color", (float*)&entities.name_color);
            }
            
            ImGui::EndTabItem();
        }
        
        // Map ESP Tab
        if (ImGui::BeginTabItem("Map")) {
            ImGui::Checkbox("Enable Map ESP", &map.enabled);
            
            if (map.enabled) {
                ImGui::Separator();
                
                // Map visualization options
                ImGui::Checkbox("Show Triangles", &map.show_triangles);
                
                if (map.show_triangles) {
                    ImGui::Separator();
                    
                    ImGui::Checkbox("Wireframe Only", &map.wireframe_only);
                    ImGui::SliderFloat("Triangle Alpha", &map.triangle_alpha, 0.0f, 1.0f, "%.2f");
                    ImGui::SliderFloat("Max Distance", &map.max_distance, 5.0f, 200.0f, "%.0fm");
                    
                    ImGui::Separator();
                    
                    // Map colors
                    ImGui::Text("Colors:");
                    ImGui::ColorEdit4("Triangle Color", (float*)&map.triangle_color);
                    ImGui::ColorEdit4("Wireframe Color", (float*)&map.wireframe_color);
                    
                    ImGui::Separator();
                    
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Warning:");
                    ImGui::TextWrapped("Map triangle rendering can be performance intensive. Use low max distance and wireframe mode for better performance.");
                }
            }
            
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
}

void EspSettings::to_json(nlohmann::json& j) const {
    j["enabled"] = enabled;
    
    // Player settings
    j["player"]["enabled"] = player.enabled;
    j["player"]["enemy_only"] = player.enemy_only;
    j["player"]["box_type"] = static_cast<int>(player.box_type);
    j["player"]["player_info"] = player.player_info;
    j["player"]["visibility_check"] = player.visibility_check;
    j["player"]["max_distance"] = player.max_distance;
    j["player"]["color_visible"] = player.color_visible;
    j["player"]["color_invisible"] = player.color_invisible;
    j["player"]["name_color"] = player.name_color;
    j["player"]["box_color"] = player.box_color;
    
    // Entity settings
    j["entities"]["enabled"] = entities.enabled;
    j["entities"]["entity_types"] = entities.entity_types;
    j["entities"]["entity_info"] = entities.entity_info;
    j["entities"]["max_distance"] = entities.max_distance;
    j["entities"]["box_type"] = static_cast<int>(entities.box_type);
    j["entities"]["weapon_color"] = entities.weapon_color;
    j["entities"]["grenade_color"] = entities.grenade_color;
    j["entities"]["c4_color"] = entities.c4_color;
    j["entities"]["hostage_color"] = entities.hostage_color;
    j["entities"]["chicken_color"] = entities.chicken_color;
    j["entities"]["other_color"] = entities.other_color;
    j["entities"]["name_color"] = entities.name_color;
    
    // Map settings
    j["map"]["enabled"] = map.enabled;
    j["map"]["show_triangles"] = map.show_triangles;
    j["map"]["triangle_alpha"] = map.triangle_alpha;
    j["map"]["max_distance"] = map.max_distance;
    j["map"]["wireframe_only"] = map.wireframe_only;
    j["map"]["triangle_color"] = map.triangle_color;
    j["map"]["wireframe_color"] = map.wireframe_color;
    
    // Legacy settings for backwards compatibility
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
    
    // Load player settings
    if (j.contains("player")) {
        const auto& p = j["player"];
        if (p.contains("enabled")) player.enabled = p["enabled"];
        if (p.contains("enemy_only")) player.enemy_only = p["enemy_only"];
        if (p.contains("box_type")) player.box_type = static_cast<BOX_TYPE>(p["box_type"]);
        if (p.contains("player_info")) player.player_info = p["player_info"];
        if (p.contains("visibility_check")) player.visibility_check = p["visibility_check"];
        if (p.contains("max_distance")) player.max_distance = p["max_distance"];
        if (p.contains("color_visible") && p["color_visible"].is_array() && p["color_visible"].size() == 4) {
            player.color_visible = p["color_visible"];
        }
        if (p.contains("color_invisible") && p["color_invisible"].is_array() && p["color_invisible"].size() == 4) {
            player.color_invisible = p["color_invisible"];
        }
        if (p.contains("name_color") && p["name_color"].is_array() && p["name_color"].size() == 4) {
            player.name_color = p["name_color"];
        }
        if (p.contains("box_color") && p["box_color"].is_array() && p["box_color"].size() == 4) {
            player.box_color = p["box_color"];
        }
    }
    
    // Load entity settings
    if (j.contains("entities")) {
        const auto& e = j["entities"];
        if (e.contains("enabled")) entities.enabled = e["enabled"];
        if (e.contains("entity_types")) entities.entity_types = e["entity_types"];
        if (e.contains("entity_info")) entities.entity_info = e["entity_info"];
        if (e.contains("max_distance")) entities.max_distance = e["max_distance"];
        if (e.contains("box_type")) entities.box_type = static_cast<BOX_TYPE>(e["box_type"]);
        if (e.contains("weapon_color") && e["weapon_color"].is_array() && e["weapon_color"].size() == 4) {
            entities.weapon_color = e["weapon_color"];
        }
        if (e.contains("grenade_color") && e["grenade_color"].is_array() && e["grenade_color"].size() == 4) {
            entities.grenade_color = e["grenade_color"];
        }
        if (e.contains("c4_color") && e["c4_color"].is_array() && e["c4_color"].size() == 4) {
            entities.c4_color = e["c4_color"];
        }
        if (e.contains("hostage_color") && e["hostage_color"].is_array() && e["hostage_color"].size() == 4) {
            entities.hostage_color = e["hostage_color"];
        }
        if (e.contains("chicken_color") && e["chicken_color"].is_array() && e["chicken_color"].size() == 4) {
            entities.chicken_color = e["chicken_color"];
        }
        if (e.contains("other_color") && e["other_color"].is_array() && e["other_color"].size() == 4) {
            entities.other_color = e["other_color"];
        }
        if (e.contains("name_color") && e["name_color"].is_array() && e["name_color"].size() == 4) {
            entities.name_color = e["name_color"];
        }
    }
    
    // Load map settings
    if (j.contains("map")) {
        const auto& m = j["map"];
        if (m.contains("enabled")) map.enabled = m["enabled"];
        if (m.contains("show_triangles")) map.show_triangles = m["show_triangles"];
        if (m.contains("triangle_alpha")) map.triangle_alpha = m["triangle_alpha"];
        if (m.contains("max_distance")) map.max_distance = m["max_distance"];
        if (m.contains("wireframe_only")) map.wireframe_only = m["wireframe_only"];
        if (m.contains("triangle_color") && m["triangle_color"].is_array() && m["triangle_color"].size() == 4) {
            map.triangle_color = m["triangle_color"];
        }
        if (m.contains("wireframe_color") && m["wireframe_color"].is_array() && m["wireframe_color"].size() == 4) {
            map.wireframe_color = m["wireframe_color"];
        }
    }
    
    // Legacy settings for backwards compatibility
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