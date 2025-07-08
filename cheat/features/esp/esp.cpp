#include <features/esp/esp.hpp>
#include <features/esp/esp_utils.hpp>
#include <engine/cache/entity/entity_cache.hpp>
#include <engine/cache/world/world_cache.hpp>
#include <renderer/renderer.hpp>
#include <engine/engine.hpp>
#include <engine/physics/traceline.hpp>
#include <logger/logger.hpp>

#include <algorithm>
#include <string>

constexpr uint32_t fnv1a_32(const char* str) {
    uint32_t hash = 0x811C9DC5;
    while (*str) {
        hash ^= (uint8_t)*str++;
        hash *= 0x1000193;
    }
    return hash;
}

EspFeature::EspFeature(EntityCache* entity_cache, WorldCache* world_cache, Renderer* renderer, Engine* engine)
    : BaseFeature("ESP", entity_cache, world_cache, renderer, engine)
{
    logger::debug("ESP Feature created");
}

EspFeature::~EspFeature() {
    logger::debug("ESP Feature destroyed");
}

bool EspFeature::initialize() {
    logger::debug("Initializing ESP Feature");
    m_initialized = true;
    return true;
}

void EspFeature::shutdown() {
    logger::debug("Shutting down ESP Feature");
    BaseFeature::shutdown();
}

void EspFeature::update() {
    if (!is_feature_enabled() || !m_initialized) {
        return;
    }
}

void EspFeature::render() {
    if (!is_feature_enabled() || !m_initialized || !m_renderer || !m_engine || !m_entity_cache) {
        return;
    }

    if (m_settings.smoke.enabled) {
        RenderSmokes();
    }

    if (m_settings.player.enabled) {
        RenderPlayers();
    }
    
    if (m_settings.entities.enabled) {
        RenderWorldEntities();
    }
    
    if (m_settings.map.enabled && m_settings.map.show_triangles) {
        RenderMapTriangles();
    }
}

void EspFeature::RenderPlayers() {
    auto& players = m_entity_cache->get_players();
    if (players.empty()) {
        return;
    }

    Player* local_player = m_entity_cache->get_local_player();
    if (!local_player) {
        return;
    }

	auto traceline_manager = m_engine->get_traceline_manager();
    if (!traceline_manager) {
        logger::error("Traceline manager is not available");
        return;
	}

    const auto& view_matrix = m_engine->get_view_matrix();

    for (auto& player : players) {
        if (player.health <= 0 || player.origin == Vector3(0, 0, 0))
            continue;

        if (player.instance == local_player->instance)
			continue;

        if (m_settings.player.enemy_only && player.team == local_player->team)
            continue;

		Vector3 local_eye_pos = local_player->GetBone(BONE_DEF::HEAD).position;
		Vector3 target_eye_pos = player.GetBone(BONE_DEF::HEAD).position;

        player.is_visible = traceline_manager->is_visible(local_eye_pos, target_eye_pos);

        float distance = local_player ?
            player.origin.distance_to(local_player->origin) * 0.1f : 0.0f;
            
        if (distance > m_settings.player.max_distance)
            continue;

        ImRect box = Drawing::GetBoundingBox(player.bounds.mins, player.bounds.maxs, view_matrix);
        if (box.Min.x == 0 &&
            box.Min.y == 0 &&
            box.Max.x == 0 &&
            box.Max.y == 0)
            continue;

        ImColor player_color = (m_settings.player.visibility_check && !player.is_visible)
            ? m_settings.player.color_invisible
            : m_settings.player.color_visible;
        
        // Use dedicated box color for boxes, not visibility-based color
        ImColor box_color = m_settings.player.box_color;

        bool should_draw_details = (distance < 100.f);

        // Draw boxes
        switch (m_settings.player.box_type)
        {
        case BOX_TYPE::BOX_NONE:
            break;
        case BOX_TYPE::BOX_2D:
            DrawRectWithOptions(m_renderer,
                Vector2(box.Min.x, box.Min.y),
                Vector2(box.Max.x, box.Max.y),
                box_color,
                1.f,
                false, // corners_only
                Vector2(0, 0), // corner_size
                m_settings.player.box_shadow, // draw_shadow
                m_settings.player.box_shadow_offset, // shadow_offset
                m_settings.player.box_shadow_color // shadow_color
            );
            break;
        case BOX_TYPE::BOX_2D_CORNER:
            DrawRectWithOptions(m_renderer,
                Vector2(box.Min.x, box.Min.y),
                Vector2(box.Max.x, box.Max.y),
                box_color,
                1.f,
                true, // corners_only
                Vector2(
                    (box.Max.x - box.Min.x) * 0.2f,
                    (box.Max.y - box.Min.y) * 0.2f
                ), // corner_size
                m_settings.player.box_shadow, // draw_shadow
                m_settings.player.box_shadow_offset, // shadow_offset
                m_settings.player.box_shadow_color // shadow_color
            );
            break;
        case BOX_TYPE::BOX_2D_FILLED:
            DrawRectWithOptions(m_renderer,
                Vector2(box.Min.x, box.Min.y),
                Vector2(box.Max.x, box.Max.y),
                box_color,
                1.f,
                false, // corners_only
                Vector2(0, 0), // corner_size
                m_settings.player.box_shadow, // draw_shadow
                m_settings.player.box_shadow_offset, // shadow_offset
                m_settings.player.box_shadow_color, // shadow_color
                true, // filled
                m_settings.player.box_fill_color // fill_color
            );
            break;
        default:
            break;
        }

        // Draw player name and distance
        if (m_settings.player.player_info & PLAYER_INFO_NAME) {
            std::string formatted_name = player.player_name;
            if (m_settings.player.player_info & PLAYER_INFO_DISTANCE) {
                formatted_name += " - " + std::to_string(static_cast<int>(distance)) + "m";
            }

            ImVec2 player_name_size = ImGui::CalcTextSize(formatted_name.c_str());
            float center_x = (box.Min.x + box.Max.x - player_name_size.x) * 0.5f;
            DrawString(m_renderer,
                Vector2(center_x, box.Min.y - player_name_size.y - 2.0f),
                formatted_name.c_str(),
                m_settings.player.name_color
            );
        }

        // Draw weapon name
        if (m_settings.player.player_info & PLAYER_INFO_WEAPON) {
            ImVec2 weapon_name_size = ImGui::CalcTextSize(player.weapon_name.c_str());
            float center_x = (box.Min.x + box.Max.x - weapon_name_size.x) * 0.5f;
            DrawString(m_renderer,
                Vector2(center_x, box.Max.y + 2.0f),
                player.weapon_name.c_str(),
                m_settings.player.name_color
            );
        }

        // Draw skeleton
        if (m_settings.player.player_info & PLAYER_INFO_SKELETON) {
            for (auto& bone_connection : BONE_CONNECTIONS)
            {
                int bone1_idx = static_cast<int>(bone_connection.first);
                int bone2_idx = static_cast<int>(bone_connection.second);

                if (bone1_idx < 0 || bone1_idx >= Player::MAX_BONES ||
                    bone2_idx < 0 || bone2_idx >= Player::MAX_BONES)
                    continue;

                const PlayerBone& bone1 = player.bones[bone1_idx];
                const PlayerBone& bone2 = player.bones[bone2_idx];

                if (bone1.position == Vector3(0, 0, 0) || bone2.position == Vector3(0, 0, 0))
                    continue;

                if (bone1.position.distance_to(player.origin) > 100.f || bone2.position.distance_to(player.origin) > 100.f)
                    continue;

                Vector2 bone1_screen_pos;
                Vector2 bone2_screen_pos;

                if (!m_engine->world_to_screen(bone1.position, bone1_screen_pos))
                    continue;
                if (!m_engine->world_to_screen(bone2.position, bone2_screen_pos))
                    continue;

				bool is_bone1_visible = traceline_manager->is_visible(local_eye_pos, bone1.position);
				bool is_bone2_visible = traceline_manager->is_visible(local_eye_pos, bone2.position);

                ImColor bone_color = (is_bone1_visible && is_bone2_visible) ?
                    m_settings.player.color_visible : m_settings.player.color_invisible;

                DrawLine(m_renderer, bone1_screen_pos, bone2_screen_pos, 1.f, bone_color);
            }
        }

        // Draw health bar
        if (m_settings.player.player_info & PLAYER_INFO_HEALTHBAR) {
            float filled_height = (box.Max.y - box.Min.y) * (player.health / 100.0f);
            ImVec4 health_bar_color = Utils::GetHealthBarColor(player.health);

            // Background
            DrawRect(m_renderer,
                Vector2(box.Min.x - 6.0f, box.Min.y),
                Vector2(box.Min.x - 2.0f, box.Max.y),
                ImColor(0, 0, 0, 255),
                1.0f
            );

            // Health fill
            DrawFilledRect(m_renderer,
                Vector2(box.Min.x - 5.0f, box.Max.y - filled_height),
                Vector2(box.Min.x - 3.0f, box.Max.y),
                health_bar_color
            );

            if (should_draw_details) {
                std::string health_text = std::to_string(player.health);
                ImVec2 text_size = ImGui::CalcTextSize(health_text.c_str());
                float text_y = box.Max.y - filled_height;
                DrawString(m_renderer,
                    Vector2(box.Min.x - 8.0f - text_size.x, text_y),
                    health_text.c_str(),
                    ImColor(255, 255, 255, 255)
                );
            }
        }

        // Draw armor bar
        if (m_settings.player.player_info & PLAYER_INFO_ARMORBAR) {
            if (player.armor > 0) {
                float armor_height = (box.Max.y - box.Min.y) * (player.armor / 100.0f);
                const ImVec4 armor_bar_color = ImColor(0, 0, 255, 255);

                // Background
                DrawRect(m_renderer,
                    Vector2(box.Max.x + 2.0f, box.Min.y),
                    Vector2(box.Max.x + 6.0f, box.Max.y),
                    ImColor(0, 0, 0, 255),
                    1.0f
                );

                // Armor fill
                DrawFilledRect(m_renderer,
                    Vector2(box.Max.x + 3.0f, box.Max.y - armor_height),
                    Vector2(box.Max.x + 5.0f, box.Max.y),
                    armor_bar_color
                );
            }
        }

        // Draw indicators
        float indicator_y = box.Min.y;
        
        if (m_settings.player.player_info & PLAYER_INFO_C4_CARRIER) {
            if (player.has_c4) {
                const char* c4_text = "C4";
                DrawString(m_renderer,
                    Vector2(box.Max.x + 8.0f, indicator_y),
                    c4_text,
                    ImColor(255, 0, 0, 255)
                );
                indicator_y += ImGui::CalcTextSize(c4_text).y + 2.0f;
            }
        }

        if (m_settings.player.player_info & PLAYER_INFO_DEFUSER) {
            if (player.has_defuser) {
                const char* defuser_text = "DEFUSER";
                DrawString(m_renderer,
                    Vector2(box.Max.x + 8.0f, indicator_y),
                    defuser_text,
                    ImColor(0, 255, 0, 255)
                );
                indicator_y += ImGui::CalcTextSize(defuser_text).y + 2.0f;
            }
        }

        if (m_settings.player.player_info & PLAYER_INFO_HELMET) {
            if (player.has_helmet) {
                const char* helmet_text = "HELMET";
                DrawString(m_renderer,
                    Vector2(box.Max.x + 8.0f, indicator_y),
                    helmet_text,
                    ImColor(255, 255, 0, 255)
                );
                indicator_y += ImGui::CalcTextSize(helmet_text).y + 2.0f;
            }
        }
    }
}

void EspFeature::RenderWorldEntities() {
    auto& entities = m_entity_cache->get_entities();
    if (entities.empty()) {
        return;
    }

    Player* local_player = m_entity_cache->get_local_player();
    if (!local_player) {
        return;
    }

    const auto& view_matrix = m_engine->get_view_matrix();

    for (const auto& entity : entities) {
        if (entity.origin == Vector3(0, 0, 0) || entity.classname.empty()) {
            continue;
        }

        // Skip player entities
        if (entity.classname_hash == fnv1a_32("CCSPlayerController") || 
            entity.classname_hash == fnv1a_32("C_CSPlayerPawn") ||
            entity.classname_hash == fnv1a_32("C_CSPlayerPawnBase") ||
            entity.classname_hash == fnv1a_32("C_BasePlayerController") ||
            entity.classname_hash == fnv1a_32("C_BasePlayerPawn")) {
            continue;
        }

        // Check entity type filters
        bool should_render = false;
        if ((m_settings.entities.entity_types & ENTITY_TYPE_WEAPONS) && IsWeaponEntity(entity.classname_hash)) {
            should_render = true;
        }
        if ((m_settings.entities.entity_types & ENTITY_TYPE_GRENADES) && IsGrenadeEntity(entity.classname_hash)) {
            should_render = true;
        }
        if ((m_settings.entities.entity_types & ENTITY_TYPE_C4) && entity.classname_hash == fnv1a_32("C_C4")) {
            should_render = true;
        }
        if ((m_settings.entities.entity_types & ENTITY_TYPE_HOSTAGES) && IsHostageEntity(entity.classname_hash)) {
            should_render = true;
        }
        if ((m_settings.entities.entity_types & ENTITY_TYPE_CHICKENS) && IsChickenEntity(entity.classname_hash)) {
            should_render = true;
        }
        if ((m_settings.entities.entity_types & ENTITY_TYPE_OTHER) && 
            !IsWeaponEntity(entity.classname_hash) && !IsGrenadeEntity(entity.classname_hash) && 
            entity.classname_hash != fnv1a_32("C_C4") && !IsHostageEntity(entity.classname_hash) && 
            !IsChickenEntity(entity.classname_hash)) {
            should_render = true;
        }

        if (!should_render) {
            continue;
        }

        float distance = entity.origin.distance_to(local_player->origin) * 0.1f;
        if (distance > m_settings.entities.max_distance) {
            continue;
        }

        Vector2 screen_pos;
        if (!m_engine->world_to_screen(entity.origin, screen_pos)) {
            continue;
        }

        ImColor entity_color = GetEntityColor(entity.classname_hash);
        std::string display_name = GetEntityDisplayName(entity.classname_hash);

        // Draw entity box if enabled
        if (m_settings.entities.entity_info & ENTITY_INFO_BOX) {
            // Create a simple bounding box around the entity position
            Vector3 mins = entity.origin - Vector3(10, 10, 10);
            Vector3 maxs = entity.origin + Vector3(10, 10, 10);
            
            ImRect box = Drawing::GetBoundingBox(mins, maxs, view_matrix);
            if (box.Min.x != 0 || box.Min.y != 0 || box.Max.x != 0 || box.Max.y != 0) {
                switch (m_settings.entities.box_type) {
                case BOX_TYPE::BOX_2D:
                    DrawRectWithOptions(m_renderer,
                        Vector2(box.Min.x, box.Min.y),
                        Vector2(box.Max.x, box.Max.y),
                        entity_color,
                        1.f,
                        false, // corners_only
                        Vector2(0, 0), // corner_size
                        m_settings.entities.box_shadow, // draw_shadow
                        m_settings.entities.box_shadow_offset, // shadow_offset
                        m_settings.entities.box_shadow_color // shadow_color
                    );
                    break;
                case BOX_TYPE::BOX_2D_CORNER:
                    DrawRectWithOptions(m_renderer,
                        Vector2(box.Min.x, box.Min.y),
                        Vector2(box.Max.x, box.Max.y),
                        entity_color,
                        1.f,
                        true, // corners_only
                        Vector2(
                            (box.Max.x - box.Min.x) * 0.2f,
                            (box.Max.y - box.Min.y) * 0.2f
                        ), // corner_size
                        m_settings.entities.box_shadow, // draw_shadow
                        m_settings.entities.box_shadow_offset, // shadow_offset
                        m_settings.entities.box_shadow_color // shadow_color
                    );
                    break;
                case BOX_TYPE::BOX_2D_FILLED:
                    DrawRectWithOptions(m_renderer,
                        Vector2(box.Min.x, box.Min.y),
                        Vector2(box.Max.x, box.Max.y),
                        entity_color,
                        1.f,
                        false, // corners_only
                        Vector2(0, 0), // corner_size
                        m_settings.entities.box_shadow, // draw_shadow
                        m_settings.entities.box_shadow_offset, // shadow_offset
                        m_settings.entities.box_shadow_color, // shadow_color
                        true, // filled
                        ImColor(entity_color.Value.x, entity_color.Value.y, entity_color.Value.z, 0.3f) // fill_color (use entity color with transparency)
                    );
                    break;
                default:
                    break;
                }
            }
        }

        // Draw entity name
        if (m_settings.entities.entity_info & ENTITY_INFO_NAME) {
            std::string formatted_name = display_name;
            if (m_settings.entities.entity_info & ENTITY_INFO_DISTANCE) {
                formatted_name += " - " + std::to_string(static_cast<int>(distance)) + "m";
            }

            ImVec2 text_size = ImGui::CalcTextSize(formatted_name.c_str());
            DrawString(m_renderer,
                Vector2(screen_pos.x - text_size.x * 0.5f, screen_pos.y - text_size.y - 5.0f),
                formatted_name.c_str(),
                m_settings.entities.name_color
            );
        }
    }
}

void EspFeature::RenderSmokes()
{
    if (!m_settings.smoke.enabled) {
        return;
    }

    Player* local_player = m_entity_cache->get_local_player();
    if (!local_player) {
        return;
    }

    // Collect all voxels from all smokes with distance information
    struct VoxelRenderInfo {
        VoxelData voxel;
        float distance_to_player;
    };
    
    std::vector<VoxelRenderInfo> voxels_to_render;
    
    for (auto& smoke : m_entity_cache->get_smokes()) {
        float smoke_distance = smoke.smoke_center.distance_to(local_player->origin) * 0.1f;
        
        // Skip entire smoke if too far and distance culling is enabled
        if (m_settings.smoke.cull_by_distance && smoke_distance > m_settings.smoke.max_distance) {
            continue;
        }

        for (auto& voxel : smoke.voxels) {
            // Skip voxels with density below threshold
            if (voxel.density < m_settings.smoke.min_density_threshold) {
                continue;
            }

            float voxel_distance = voxel.world_position.distance_to(local_player->origin) * 0.1f;
            
            // Skip voxels that are too far
            if (m_settings.smoke.cull_by_distance && voxel_distance > m_settings.smoke.max_distance) {
                continue;
            }

            voxels_to_render.push_back({voxel, voxel_distance});
        }
    }

    // Sort voxels by distance if enabled (back to front for proper alpha blending)
    if (m_settings.smoke.distance_sorting) {
        std::sort(voxels_to_render.begin(), voxels_to_render.end(), 
                  [](const VoxelRenderInfo& a, const VoxelRenderInfo& b) {
                      return a.distance_to_player > b.distance_to_player;
                  });
    }

    // Render all voxels as 3D cubes
    for (const auto& voxel_info : voxels_to_render) {
        const VoxelData& voxel = voxel_info.voxel;
        
        Drawing::DrawVoxelCube(
            m_renderer,
            voxel.world_position,
            m_settings.smoke.cube_size,
            voxel.density,
            m_engine,
            m_settings.smoke.min_density_threshold,
            m_settings.smoke.max_opacity,
            m_settings.smoke.density_multiplier,
            m_settings.smoke.use_gradient_colors,
            m_settings.smoke.low_density_color,
            m_settings.smoke.high_density_color,
            m_settings.smoke.show_edges,
            m_settings.smoke.edge_thickness,
            m_settings.smoke.edge_color
        );
    }
}

void EspFeature::RenderMapTriangles() {
    if (!m_world_cache) {
        return;
    }

    Player* local_player = m_entity_cache->get_local_player();
    if (!local_player) {
        return;
    }

    const auto& triangles = m_world_cache->get_triangles();
    if (triangles.empty()) {
        return;
    }

    const auto& view_matrix = m_engine->get_view_matrix();

    for (const auto& triangle : triangles) {
        // Calculate distance to triangle center
        Vector3 center = (triangle.vertices[0] + triangle.vertices[1] + triangle.vertices[2]) / 3.0f;
        float distance = center.distance_to(local_player->origin) * 0.1f;
        
        if (distance > m_settings.map.max_distance) {
            continue;
        }

        // Convert triangle vertices to screen coordinates
        Vector2 screen_vertices[3];
        bool all_on_screen = true;
        
        for (int i = 0; i < 3; i++) {
            if (!m_engine->world_to_screen(triangle.vertices[i], screen_vertices[i])) {
                all_on_screen = false;
                break;
            }
        }

        if (!all_on_screen) {
            continue;
        }

        // Draw filled triangle if not wireframe only
        if (!m_settings.map.wireframe_only) {
            ImColor fill_color = m_settings.map.triangle_color;
            fill_color.Value.w = m_settings.map.triangle_alpha;
            
            m_renderer->get_draw_list()->AddTriangleFilled(
                ImVec2(screen_vertices[0].x, screen_vertices[0].y),
                ImVec2(screen_vertices[1].x, screen_vertices[1].y),
                ImVec2(screen_vertices[2].x, screen_vertices[2].y),
                fill_color
            );
        }

        // Draw wireframe
        ImColor wireframe_color = m_settings.map.wireframe_color;
        m_renderer->get_draw_list()->AddTriangle(
            ImVec2(screen_vertices[0].x, screen_vertices[0].y),
            ImVec2(screen_vertices[1].x, screen_vertices[1].y),
            ImVec2(screen_vertices[2].x, screen_vertices[2].y),
            wireframe_color,
            1.0f
        );
    }
}

bool EspFeature::IsWeaponEntity(uint32_t classname_hash) const {
    switch (classname_hash) {
        // Weapons
        case fnv1a_32("C_WeaponAK47"):
        case fnv1a_32("C_WeaponAWP"):
        case fnv1a_32("C_WeaponAug"):
        case fnv1a_32("C_WeaponBizon"):
        case fnv1a_32("C_WeaponElite"):
        case fnv1a_32("C_WeaponFamas"):
        case fnv1a_32("C_WeaponFiveSeven"):
        case fnv1a_32("C_WeaponG3SG1"):
        case fnv1a_32("C_WeaponGalilAR"):
        case fnv1a_32("C_WeaponGlock"):
        case fnv1a_32("C_WeaponHKP2000"):
        case fnv1a_32("C_WeaponM249"):
        case fnv1a_32("C_WeaponM4A1"):
        case fnv1a_32("C_WeaponM4A1Silencer"):
        case fnv1a_32("C_WeaponMAC10"):
        case fnv1a_32("C_WeaponMag7"):
        case fnv1a_32("C_WeaponMP5SD"):
        case fnv1a_32("C_WeaponMP7"):
        case fnv1a_32("C_WeaponMP9"):
        case fnv1a_32("C_WeaponNegev"):
        case fnv1a_32("C_WeaponNOVA"):
        case fnv1a_32("C_WeaponP90"):
        case fnv1a_32("C_WeaponP250"):
        case fnv1a_32("C_WeaponRevolver"):
        case fnv1a_32("C_WeaponSCAR20"):
        case fnv1a_32("C_WeaponSG556"):
        case fnv1a_32("C_WeaponSSG08"):
        case fnv1a_32("C_WeaponSawedoff"):
        case fnv1a_32("C_WeaponTaser"):
        case fnv1a_32("C_WeaponTec9"):
        case fnv1a_32("C_WeaponUMP45"):
        case fnv1a_32("C_WeaponUSPSilencer"):
        case fnv1a_32("C_WeaponXM1014"):
        case fnv1a_32("C_Knife"):
        case fnv1a_32("C_DEagle"):
        case fnv1a_32("C_AK47"):
        case fnv1a_32("C_CZ75a"):
            return true;
        default:
            return false;
    }
}

bool EspFeature::IsGrenadeEntity(uint32_t classname_hash) const {
    switch (classname_hash) {
        case fnv1a_32("C_FlashbangProjectile"):
        case fnv1a_32("C_SmokeGrenadeProjectile"):
        case fnv1a_32("C_HEGrenadeProjectile"):
        case fnv1a_32("C_DecoyProjectile"):
        case fnv1a_32("C_MolotovProjectile"):
        case fnv1a_32("C_BaseCSGrenadeProjectile"):
        case fnv1a_32("C_Flashbang"):
        case fnv1a_32("C_SmokeGrenade"):
        case fnv1a_32("C_HEGrenade"):
        case fnv1a_32("C_DecoyGrenade"):
        case fnv1a_32("C_MolotovGrenade"):
        case fnv1a_32("C_IncendiaryGrenade"):
        case fnv1a_32("C_Inferno"):
            return true;
        default:
            return false;
    }
}

bool EspFeature::IsHostageEntity(uint32_t classname_hash) const {
    switch (classname_hash) {
        case fnv1a_32("C_Hostage"):
        case fnv1a_32("C_HostageCarriableProp"):
            return true;
        default:
            return false;
    }
}

bool EspFeature::IsChickenEntity(uint32_t classname_hash) const {
    switch (classname_hash) {
        case fnv1a_32("C_Chicken"):
            return true;
        default:
            return false;
    }
}

ImColor EspFeature::GetEntityColor(uint32_t classname_hash) const {
    if (IsWeaponEntity(classname_hash)) {
        return m_settings.entities.weapon_color;
    }
    if (IsGrenadeEntity(classname_hash)) {
        return m_settings.entities.grenade_color;
    }
    if (classname_hash == fnv1a_32("C_C4") || classname_hash == fnv1a_32("C_PlantedC4")) {
        return m_settings.entities.c4_color;
    }
    if (IsHostageEntity(classname_hash)) {
        return m_settings.entities.hostage_color;
    }
    if (IsChickenEntity(classname_hash)) {
        return m_settings.entities.chicken_color;
    }
    return m_settings.entities.other_color;
}

std::string EspFeature::GetEntityDisplayName(uint32_t classname_hash) const {
    switch (classname_hash) {
        // C4 and Bomb
        case fnv1a_32("C_C4"):
            return "C4";
        case fnv1a_32("C_PlantedC4"):
            return "PLANTED C4";
            
        // Grenades
        case fnv1a_32("C_FlashbangProjectile"):
            return "FLASHBANG";
        case fnv1a_32("C_SmokeGrenadeProjectile"):
            return "SMOKE GRENADE";
        case fnv1a_32("C_HEGrenadeProjectile"):
            return "HE GRENADE";
        case fnv1a_32("C_DecoyProjectile"):
            return "DECOY GRENADE";
        case fnv1a_32("C_MolotovProjectile"):
            return "MOLOTOV";
        case fnv1a_32("C_Inferno"):
            return "FIRE";
        case fnv1a_32("C_Flashbang"):
            return "FLASHBANG";
        case fnv1a_32("C_SmokeGrenade"):
            return "SMOKE GRENADE";
        case fnv1a_32("C_HEGrenade"):
            return "HE GRENADE";
        case fnv1a_32("C_DecoyGrenade"):
            return "DECOY GRENADE";
        case fnv1a_32("C_MolotovGrenade"):
            return "MOLOTOV";
        case fnv1a_32("C_IncendiaryGrenade"):
            return "INCENDIARY";
            
        // Weapons - Rifles
        case fnv1a_32("C_WeaponAK47"):
        case fnv1a_32("C_AK47"):
            return "AK-47";
        case fnv1a_32("C_WeaponM4A1"):
            return "M4A1";
        case fnv1a_32("C_WeaponM4A1Silencer"):
            return "M4A1-S";
        case fnv1a_32("C_WeaponAug"):
            return "AUG";
        case fnv1a_32("C_WeaponFamas"):
            return "FAMAS";
        case fnv1a_32("C_WeaponGalilAR"):
            return "GALIL AR";
        case fnv1a_32("C_WeaponSG556"):
            return "SG 553";
            
        // Weapons - Sniper Rifles
        case fnv1a_32("C_WeaponAWP"):
            return "AWP";
        case fnv1a_32("C_WeaponSSG08"):
            return "SSG 08";
        case fnv1a_32("C_WeaponSCAR20"):
            return "SCAR-20";
        case fnv1a_32("C_WeaponG3SG1"):
            return "G3SG1";
            
        // Weapons - SMGs
        case fnv1a_32("C_WeaponBizon"):
            return "PP-Bizon";
        case fnv1a_32("C_WeaponMP7"):
            return "MP7";
        case fnv1a_32("C_WeaponMP9"):
            return "MP9";
        case fnv1a_32("C_WeaponMAC10"):
            return "MAC-10";
        case fnv1a_32("C_WeaponP90"):
            return "P90";
        case fnv1a_32("C_WeaponUMP45"):
            return "UMP-45";
        case fnv1a_32("C_WeaponMP5SD"):
            return "MP5-SD";
            
        // Weapons - Shotguns
        case fnv1a_32("C_WeaponNOVA"):
            return "Nova";
        case fnv1a_32("C_WeaponXM1014"):
            return "XM1014";
        case fnv1a_32("C_WeaponSawedoff"):
            return "Sawed-Off";
        case fnv1a_32("C_WeaponMag7"):
            return "MAG-7";
            
        // Weapons - Pistols
        case fnv1a_32("C_WeaponGlock"):
            return "Glock-18";
        case fnv1a_32("C_WeaponUSPSilencer"):
            return "USP-S";
        case fnv1a_32("C_WeaponHKP2000"):
            return "P2000";
        case fnv1a_32("C_WeaponElite"):
            return "Dual Berettas";
        case fnv1a_32("C_WeaponP250"):
            return "P250";
        case fnv1a_32("C_WeaponTec9"):
            return "Tec-9";
        case fnv1a_32("C_WeaponFiveSeven"):
            return "Five-SeveN";
        case fnv1a_32("C_WeaponRevolver"):
            return "R8 Revolver";
        case fnv1a_32("C_DEagle"):
            return "Desert Eagle";
        case fnv1a_32("C_CZ75a"):
            return "CZ75-Auto";
        case fnv1a_32("C_WeaponTaser"):
            return "Zeus x27";
            
        // Weapons - Machine Guns
        case fnv1a_32("C_WeaponM249"):
            return "M249";
        case fnv1a_32("C_WeaponNegev"):
            return "Negev";
            
        // Weapons - Melee
        case fnv1a_32("C_Knife"):
            return "Knife";
            
        // Hostages
        case fnv1a_32("C_Hostage"):
            return "HOSTAGE";
        case fnv1a_32("C_HostageCarriableProp"):
            return "HOSTAGE";
            
        // Chicken
        case fnv1a_32("C_Chicken"):
            return "CHICKEN";
            
        default:
            return "UNKNOWN";
    }
}
