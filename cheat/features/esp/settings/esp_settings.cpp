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
                const char* box_type_items[] = { "None", "2D", "2D Corner", "2D Filled" };
                int current_box_type = static_cast<int>(player.box_type);
                if (ImGui::Combo("##PlayerBoxType", &current_box_type, box_type_items, IM_ARRAYSIZE(box_type_items))) {
                    player.box_type = static_cast<BOX_TYPE>(current_box_type);
                }
                
                if (player.box_type != BOX_TYPE::BOX_NONE) {
                    ImGui::Checkbox("Box Shadow", &player.box_shadow);
                    if (player.box_shadow) {
                        ImGui::SliderFloat("Shadow Offset", &player.box_shadow_offset, 0.5f, 3.0f, "%.1f");
                    }
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
                
                // Box colors section
                ImGui::Text("Box Colors:");
                ImGui::ColorEdit4("Box Outline", (float*)&player.box_color);
                if (player.box_type == BOX_TYPE::BOX_2D_FILLED) {
                    ImGui::ColorEdit4("Box Fill", (float*)&player.box_fill_color);
                }
                if (player.box_shadow && player.box_type != BOX_TYPE::BOX_NONE) {
                    ImGui::ColorEdit4("Box Shadow", (float*)&player.box_shadow_color);
                }
                
                ImGui::Separator();
                
                // Visibility colors section
                ImGui::Text("Visibility Colors (for skeleton, etc.):");
                ImGui::ColorEdit4("Visible Color", (float*)&player.color_visible);
                ImGui::ColorEdit4("Invisible Color", (float*)&player.color_invisible);
                
                ImGui::Separator();
                
                // Text colors
                ImGui::Text("Text Colors:");
                ImGui::ColorEdit4("Name Color", (float*)&player.name_color);
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
                const char* entity_box_type_items[] = { "None", "2D", "2D Corner", "2D Filled" };
                int current_entity_box_type = static_cast<int>(entities.box_type);
                if (ImGui::Combo("##EntityBoxType", &current_entity_box_type, entity_box_type_items, IM_ARRAYSIZE(entity_box_type_items))) {
                    entities.box_type = static_cast<BOX_TYPE>(current_entity_box_type);
                }
                
                if (entities.box_type != BOX_TYPE::BOX_NONE) {
                    ImGui::Checkbox("Box Shadow##Entity", &entities.box_shadow);
                    if (entities.box_shadow) {
                        ImGui::SliderFloat("Shadow Offset##Entity", &entities.box_shadow_offset, 0.5f, 3.0f, "%.1f");
                    }
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
                if (entities.box_shadow && entities.box_type != BOX_TYPE::BOX_NONE) {
                    ImGui::ColorEdit4("Box Shadow Color##Entity", (float*)&entities.box_shadow_color);
                }
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
                }
            }
            
            ImGui::EndTabItem();
        }
        
        // Smoke ESP Tab
        if (ImGui::BeginTabItem("Smoke")) {
            ImGui::Checkbox("Enable Smoke ESP", &smoke.enabled);
            
            if (smoke.enabled) {
                ImGui::Separator();
                
                // 3D Cube rendering settings
                ImGui::Text("3D Cube Settings:");
                ImGui::SliderFloat("Cube Size", &smoke.cube_size, 5.0f, 50.0f, "%.1f");
                ImGui::SliderFloat("Max Distance", &smoke.max_distance, 50.0f, 500.0f, "%.0fm");
                ImGui::SliderFloat("Min Density", &smoke.min_density_threshold, 0.001f, 0.1f, "%.3f");
                ImGui::SliderFloat("Max Opacity", &smoke.max_opacity, 0.1f, 1.0f, "%.2f");
                ImGui::SliderFloat("Density Multiplier", &smoke.density_multiplier, 0.1f, 5.0f, "%.1f");
                
                ImGui::Separator();
                
                // Color settings
                ImGui::Text("Color Settings:");
                ImGui::Checkbox("Use Gradient Colors", &smoke.use_gradient_colors);
                ImGui::ColorEdit4("Low Density Color", (float*)&smoke.low_density_color);
                if (smoke.use_gradient_colors) {
                    ImGui::ColorEdit4("High Density Color", (float*)&smoke.high_density_color);
                }
                
                ImGui::Separator();
                
                // Edge settings
                ImGui::Text("Edge Settings:");
                ImGui::Checkbox("Show Edges", &smoke.show_edges);
                if (smoke.show_edges) {
                    ImGui::SliderFloat("Edge Thickness", &smoke.edge_thickness, 0.5f, 3.0f, "%.1f");
                    ImGui::ColorEdit4("Edge Color", (float*)&smoke.edge_color);
                }
                
                ImGui::Separator();
                
                // Rendering options
                ImGui::Text("Rendering Options:");
                ImGui::Checkbox("Distance Sorting", &smoke.distance_sorting);
                ImGui::Checkbox("Cull by Distance", &smoke.cull_by_distance);
                
                ImGui::Separator();
                
                // Performance note
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Note: 3D smoke rendering may impact performance");
                ImGui::Text("Reduce max distance or increase min density for better FPS");
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
    j["player"]["box_shadow"] = player.box_shadow;
    j["player"]["box_shadow_offset"] = player.box_shadow_offset;
    j["player"]["color_visible"] = player.color_visible;
    j["player"]["color_invisible"] = player.color_invisible;
    j["player"]["name_color"] = player.name_color;
    j["player"]["box_color"] = player.box_color;
    j["player"]["box_fill_color"] = player.box_fill_color;
    j["player"]["box_shadow_color"] = player.box_shadow_color;
    
    // Entity settings
    j["entities"]["enabled"] = entities.enabled;
    j["entities"]["entity_types"] = entities.entity_types;
    j["entities"]["entity_info"] = entities.entity_info;
    j["entities"]["max_distance"] = entities.max_distance;
    j["entities"]["box_type"] = static_cast<int>(entities.box_type);
    j["entities"]["box_shadow"] = entities.box_shadow;
    j["entities"]["box_shadow_offset"] = entities.box_shadow_offset;
    j["entities"]["weapon_color"] = entities.weapon_color;
    j["entities"]["grenade_color"] = entities.grenade_color;
    j["entities"]["c4_color"] = entities.c4_color;
    j["entities"]["hostage_color"] = entities.hostage_color;
    j["entities"]["chicken_color"] = entities.chicken_color;
    j["entities"]["other_color"] = entities.other_color;
    j["entities"]["name_color"] = entities.name_color;
    j["entities"]["box_shadow_color"] = entities.box_shadow_color;
    
    // Map settings
    j["map"]["enabled"] = map.enabled;
    j["map"]["show_triangles"] = map.show_triangles;
    j["map"]["triangle_alpha"] = map.triangle_alpha;
    j["map"]["max_distance"] = map.max_distance;
    j["map"]["wireframe_only"] = map.wireframe_only;
    j["map"]["triangle_color"] = map.triangle_color;
    j["map"]["wireframe_color"] = map.wireframe_color;
    
    // Smoke settings
    j["smoke"]["enabled"] = smoke.enabled;
    j["smoke"]["cube_size"] = smoke.cube_size;
    j["smoke"]["max_distance"] = smoke.max_distance;
    j["smoke"]["min_density_threshold"] = smoke.min_density_threshold;
    j["smoke"]["max_opacity"] = smoke.max_opacity;
    j["smoke"]["density_multiplier"] = smoke.density_multiplier;
    j["smoke"]["use_gradient_colors"] = smoke.use_gradient_colors;
    j["smoke"]["low_density_color"] = smoke.low_density_color;
    j["smoke"]["high_density_color"] = smoke.high_density_color;
    j["smoke"]["show_edges"] = smoke.show_edges;
    j["smoke"]["edge_thickness"] = smoke.edge_thickness;
    j["smoke"]["edge_color"] = smoke.edge_color;
    j["smoke"]["distance_sorting"] = smoke.distance_sorting;
    j["smoke"]["cull_by_distance"] = smoke.cull_by_distance;
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
        if (p.contains("box_shadow")) player.box_shadow = p["box_shadow"];
        if (p.contains("box_shadow_offset")) player.box_shadow_offset = p["box_shadow_offset"];
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
        if (p.contains("box_fill_color") && p["box_fill_color"].is_array() && p["box_fill_color"].size() == 4) {
            player.box_fill_color = p["box_fill_color"];
        }
        if (p.contains("box_shadow_color") && p["box_shadow_color"].is_array() && p["box_shadow_color"].size() == 4) {
            player.box_shadow_color = p["box_shadow_color"];
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
        if (e.contains("box_shadow")) entities.box_shadow = e["box_shadow"];
        if (e.contains("box_shadow_offset")) entities.box_shadow_offset = e["box_shadow_offset"];
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
        if (e.contains("box_shadow_color") && e["box_shadow_color"].is_array() && e["box_shadow_color"].size() == 4) {
            entities.box_shadow_color = e["box_shadow_color"];
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
    
    // Load smoke settings
    if (j.contains("smoke")) {
        const auto& s = j["smoke"];
        if (s.contains("enabled")) smoke.enabled = s["enabled"];
        if (s.contains("cube_size")) smoke.cube_size = s["cube_size"];
        if (s.contains("max_distance")) smoke.max_distance = s["max_distance"];
        if (s.contains("min_density_threshold")) smoke.min_density_threshold = s["min_density_threshold"];
        if (s.contains("max_opacity")) smoke.max_opacity = s["max_opacity"];
        if (s.contains("density_multiplier")) smoke.density_multiplier = s["density_multiplier"];
        if (s.contains("use_gradient_colors")) smoke.use_gradient_colors = s["use_gradient_colors"];
        if (s.contains("low_density_color") && s["low_density_color"].is_array() && s["low_density_color"].size() == 4) {
            smoke.low_density_color = s["low_density_color"];
        }
        if (s.contains("high_density_color") && s["high_density_color"].is_array() && s["high_density_color"].size() == 4) {
            smoke.high_density_color = s["high_density_color"];
        }
        if (s.contains("show_edges")) smoke.show_edges = s["show_edges"];
        if (s.contains("edge_thickness")) smoke.edge_thickness = s["edge_thickness"];
        if (s.contains("edge_color") && s["edge_color"].is_array() && s["edge_color"].size() == 4) {
            smoke.edge_color = s["edge_color"];
        }
        if (s.contains("distance_sorting")) smoke.distance_sorting = s["distance_sorting"];
        if (s.contains("cull_by_distance")) smoke.cull_by_distance = s["cull_by_distance"];
    }
} 