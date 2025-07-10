#pragma once
#include <features/settings/base_settings.hpp>

#include <imgui.h>

enum class BOX_TYPE {
    BOX_NONE,
    BOX_2D,
    BOX_2D_CORNER,
    BOX_2D_FILLED
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

enum ENTITY_INFO {
    ENTITY_INFO_NAME = 1 << 0,
    ENTITY_INFO_DISTANCE = 1 << 1,
    ENTITY_INFO_BOX = 1 << 2
};

enum ENTITY_TYPE {
    ENTITY_TYPE_WEAPONS = 1 << 0,
    ENTITY_TYPE_GRENADES = 1 << 1,
    ENTITY_TYPE_C4 = 1 << 2,
    ENTITY_TYPE_HOSTAGES = 1 << 3,
    ENTITY_TYPE_CHICKENS = 1 << 4,
    ENTITY_TYPE_OTHER = 1 << 5
};

class EspSettings : public BaseSettings {
public:
    EspSettings() : BaseSettings("ESP") {}
    
    void render_imgui() override;
    void to_json(nlohmann::json& j) const override;
    void from_json(const nlohmann::json& j) override;
    
    // Global ESP control
    bool enabled = true;
    
    // Player ESP Settings
    struct PlayerSettings {
        bool enabled = true;
        bool team_check = true;
        BOX_TYPE box_type = BOX_TYPE::BOX_2D;
        int player_info = PLAYER_INFO_NAME | PLAYER_INFO_WEAPON | PLAYER_INFO_HEALTHBAR | PLAYER_INFO_SKELETON;
        bool visibility_check = true;
        float max_distance = 500.0f;
        bool box_shadow = true;
        float box_shadow_offset = 1.0f;
        
        ImColor color_visible = ImColor(0.0f, 1.0f, 0.0f, 1.0f);
        ImColor color_invisible = ImColor(1.0f, 0.0f, 0.0f, 1.0f);
        ImColor name_color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
        ImColor box_color = ImColor(1.0f, 1.0f, 1.0f, 1.0f); // White boxes by default
        ImColor box_fill_color = ImColor(1.0f, 1.0f, 1.0f, 0.1f); // Very transparent white fill
        ImColor box_shadow_color = ImColor(0.0f, 0.0f, 0.0f, 0.8f);
    } player;
    
    // World Entity ESP Settings
    struct EntitySettings {
        bool enabled = true;
        int entity_types = ENTITY_TYPE_WEAPONS | ENTITY_TYPE_GRENADES | ENTITY_TYPE_C4;
        int entity_info = ENTITY_INFO_NAME | ENTITY_INFO_BOX;
        float max_distance = 200.0f;
        BOX_TYPE box_type = BOX_TYPE::BOX_2D;
        bool box_shadow = true;
        float box_shadow_offset = 1.0f;
        
        ImColor weapon_color = ImColor(1.0f, 1.0f, 1.0f, 1.0f); // White for weapons
        ImColor grenade_color = ImColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange for grenades
        ImColor c4_color = ImColor(1.0f, 0.0f, 0.0f, 1.0f); // Red for C4
        ImColor hostage_color = ImColor(0.0f, 0.5f, 1.0f, 1.0f); // Light blue for hostages
        ImColor chicken_color = ImColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow for chickens
        ImColor other_color = ImColor(0.8f, 0.8f, 0.8f, 1.0f); // Light gray for other items
        ImColor name_color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
        ImColor box_shadow_color = ImColor(0.0f, 0.0f, 0.0f, 0.8f);
    } entities;
    
    // Map ESP Settings
    struct MapSettings {
        bool enabled = false;
        bool show_triangles = true;
        float triangle_alpha = 0.1f;
        float max_distance = 50.0f;
        bool wireframe_only = true;
        
        ImColor triangle_color = ImColor(0.0f, 1.0f, 1.0f, 0.1f);
        ImColor wireframe_color = ImColor(0.0f, 1.0f, 1.0f, 0.5f);
    } map;
    
    // Smoke ESP Settings
    struct SmokeSettings {
        bool enabled = true;
        
        // 3D Cube rendering settings
        float cube_size = 20.0f; // Size of each voxel cube
        float max_distance = 300.0f; // Maximum distance to render smoke
        float min_density_threshold = 0.01f; // Minimum density to render
        float max_opacity = 0.8f; // Maximum opacity for cubes
        float density_multiplier = 1.0f; // Multiplier for density-based alpha
        
        // Color settings
        bool use_gradient_colors = true;
        ImColor low_density_color = ImColor(0.8f, 0.8f, 0.8f, 0.3f); // Light gray for low density
        ImColor high_density_color = ImColor(0.3f, 0.3f, 0.3f, 0.8f); // Dark gray for high density
        
        // Edge settings
        bool show_edges = true;
        float edge_thickness = 1.0f;
        ImColor edge_color = ImColor(0.5f, 0.5f, 0.5f, 0.6f); // Gray edges
        
        // Rendering options
        bool distance_sorting = true; // Sort voxels by distance for proper rendering
        bool cull_by_distance = true; // Don't render very far smoke
    } smoke;
}; 