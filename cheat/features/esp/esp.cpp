#include <features/esp/esp.hpp>
#include <features/esp/esp_utils.hpp>
#include <engine/cache/entity/entity_cache.hpp>
#include <engine/cache/world/world_cache.hpp>
#include <engine/renderer/renderer.hpp>
#include <engine/engine.hpp>
#include <logger/logger.hpp>
#include <algorithm>

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

    RenderPlayers();
}

void EspFeature::RenderPlayers() {
    const auto& players = m_entity_cache->get_players();
    if (players.empty()) {
        return;
    }

    Player* local_player = m_entity_cache->get_local_player();
    if (!local_player) {
        return;
    }

    const auto& view_matrix = m_engine->get_view_matrix();

    for (const auto& player : players) {
        if (player.health <= 0 || player.origin == Vector3(0, 0, 0))
            continue;

		// Skip local player
        if (player.instance == local_player->instance)
			continue;

        // Skip teammates if enemy only is enabled
        if (m_settings.enemy_only && player.team == local_player->team)
            continue;

        // Debug draw all pointers

        ImRect box = Drawing::GetBoundingBox(player.bounds.mins, player.bounds.maxs, view_matrix);
        if (box.Min.x == 0 &&
            box.Min.y == 0 &&
            box.Max.x == 0 &&
            box.Max.y == 0)
            continue;

        ImColor player_color = (m_settings.player_visibility_check && !player.is_visible)
            ? m_settings.player_color_invisible
            : m_settings.player_color_visible;

        float distance = local_player ?
            player.origin.distance_to(local_player->origin) * 0.1f : 0.0f;

        bool should_draw_details = (distance < 100.f);

        // Draw boxes
        switch (m_settings.box_type)
        {
        case BOX_TYPE::BOX_NONE:
            break;
        case BOX_TYPE::BOX_2D:
            DrawRect(
                Vector2(box.Min.x, box.Min.y),
                Vector2(box.Max.x, box.Max.y),
                player_color,
                1.f
            );
            break;
        case BOX_TYPE::BOX_2D_CORNER:
            DrawRect(
                Vector2(box.Min.x, box.Min.y),
                Vector2(box.Max.x, box.Max.y),
                player_color,
                1.f,
                true,
                Vector2(
                    (box.Max.x - box.Min.x) * 0.2f,
                    (box.Max.y - box.Min.y) * 0.2f
                )
            );
            break;
        default:
            break;
        }

        // Draw player name and distance
        if (m_settings.player_info & PLAYER_INFO_NAME) {
            std::string formatted_name = player.player_name;
            if (m_settings.player_info & PLAYER_INFO_DISTANCE) {
                formatted_name += " - " + std::to_string(static_cast<int>(distance)) + "m";
            }

            ImVec2 player_name_size = ImGui::CalcTextSize(formatted_name.c_str());
            float center_x = (box.Min.x + box.Max.x - player_name_size.x) * 0.5f;
            DrawString(
                Vector2(center_x, box.Min.y - player_name_size.y - 2.0f),
                formatted_name.c_str(),
                m_settings.name_color
            );
        }

        // Draw weapon name
        if (m_settings.player_info & PLAYER_INFO_WEAPON) {
            ImVec2 weapon_name_size = ImGui::CalcTextSize(player.weapon_name.c_str());
            float center_x = (box.Min.x + box.Max.x - weapon_name_size.x) * 0.5f;
            DrawString(
                Vector2(center_x, box.Max.y + 2.0f),
                player.weapon_name.c_str(),
                m_settings.name_color
            );
        }

        // Draw skeleton
        if (m_settings.player_info & PLAYER_INFO_SKELETON) {
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

                DrawLine(bone1_screen_pos, bone2_screen_pos, 1.f, player_color);
            }
        }

        // Draw health bar
        if (m_settings.player_info & PLAYER_INFO_HEALTHBAR) {
            float filled_height = (box.Max.y - box.Min.y) * (player.health / 100.0f);
            ImVec4 health_bar_color = Utils::GetHealthBarColor(player.health);

            // Background
            DrawRect(
                Vector2(box.Min.x - 6.0f, box.Min.y),
                Vector2(box.Min.x - 2.0f, box.Max.y),
                ImColor(0, 0, 0, 255),
                1.0f
            );

            // Health fill
            DrawFilledRect(
                Vector2(box.Min.x - 5.0f, box.Max.y - filled_height),
                Vector2(box.Min.x - 3.0f, box.Max.y),
                health_bar_color
            );

            if (should_draw_details) {
                std::string health_text = std::to_string(player.health);
                ImVec2 text_size = ImGui::CalcTextSize(health_text.c_str());
                float text_y = box.Max.y - filled_height;
                DrawString(
                    Vector2(box.Min.x - 8.0f - text_size.x, text_y),
                    health_text.c_str(),
                    ImColor(255, 255, 255, 255)
                );
            }
        }

        // Draw armor bar
        if (m_settings.player_info & PLAYER_INFO_ARMORBAR) {
            if (player.armor > 0) {
                float armor_height = (box.Max.y - box.Min.y) * (player.armor / 100.0f);
                const ImVec4 armor_bar_color = ImColor(0, 0, 255, 255);

                // Background
                DrawRect(
                    Vector2(box.Max.x + 2.0f, box.Min.y),
                    Vector2(box.Max.x + 6.0f, box.Max.y),
                    ImColor(0, 0, 0, 255),
                    1.0f
                );

                // Armor fill
                DrawFilledRect(
                    Vector2(box.Max.x + 3.0f, box.Max.y - armor_height),
                    Vector2(box.Max.x + 5.0f, box.Max.y),
                    armor_bar_color
                );
            }
        }

        // Draw indicators
        float indicator_y = box.Min.y;
        
        if (m_settings.player_info & PLAYER_INFO_C4_CARRIER) {
            if (player.has_c4) {
                const char* c4_text = "C4";
                DrawString(
                    Vector2(box.Max.x + 8.0f, indicator_y),
                    c4_text,
                    ImColor(255, 0, 0, 255)
                );
                indicator_y += ImGui::CalcTextSize(c4_text).y + 2.0f;
            }
        }

        if (m_settings.player_info & PLAYER_INFO_DEFUSER) {
            if (player.has_defuser) {
                const char* defuser_text = "DEFUSER";
                DrawString(
                    Vector2(box.Max.x + 8.0f, indicator_y),
                    defuser_text,
                    ImColor(0, 255, 0, 255)
                );
                indicator_y += ImGui::CalcTextSize(defuser_text).y + 2.0f;
            }
        }

        if (m_settings.player_info & PLAYER_INFO_HELMET) {
            if (player.has_helmet) {
                const char* helmet_text = "HELMET";
                DrawString(
                    Vector2(box.Max.x + 8.0f, indicator_y),
                    helmet_text,
                    ImColor(255, 255, 0, 255)
                );
                indicator_y += ImGui::CalcTextSize(helmet_text).y + 2.0f;
            }
        }
    }
}
