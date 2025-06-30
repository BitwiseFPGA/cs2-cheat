#pragma once
#include <engine/sdk/math/vector.hpp>
#include <engine/sdk/math/matrix.hpp>
#include <engine/sdk/types/player_impl.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <vector>
#include <utility>

const std::pair<BONE_DEF, BONE_DEF> BONE_CONNECTIONS[] = {
    {BONE_DEF::HEAD, BONE_DEF::NECK},
    {BONE_DEF::NECK, BONE_DEF::SPINE},
    {BONE_DEF::NECK, BONE_DEF::RIGHT_SHOULDER},
    {BONE_DEF::NECK, BONE_DEF::LEFT_SHOULDER},
    {BONE_DEF::SPINE, BONE_DEF::SPINE1},
    {BONE_DEF::SPINE1, BONE_DEF::HIP},
    {BONE_DEF::LEFT_SHOULDER, BONE_DEF::LEFT_ARM},
    {BONE_DEF::LEFT_ARM, BONE_DEF::LEFT_HAND},
    {BONE_DEF::RIGHT_SHOULDER, BONE_DEF::RIGHT_ARM},
    {BONE_DEF::RIGHT_ARM, BONE_DEF::RIGHT_HAND},
    {BONE_DEF::HIP, BONE_DEF::LEFT_HIP},
    {BONE_DEF::LEFT_HIP, BONE_DEF::LEFT_KNEE},
    {BONE_DEF::LEFT_KNEE, BONE_DEF::LEFT_FOOT},
    {BONE_DEF::HIP, BONE_DEF::RIGHT_HIP},
    {BONE_DEF::RIGHT_HIP, BONE_DEF::RIGHT_KNEE},
    {BONE_DEF::RIGHT_KNEE, BONE_DEF::RIGHT_FOOT}
};

namespace Utils {
    inline ImVec4 GetHealthBarColor(int health) {
        if (health > 75) return ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
        if (health > 50) return ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
        if (health > 25) return ImVec4(1.0f, 0.5f, 0.0f, 1.0f); // Orange
        return ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
    }
}

namespace Drawing {
    inline ImDrawList* GetDrawList() {
        return ImGui::GetBackgroundDrawList();
    }

    inline ImRect GetBoundingBox(const Vector3& mins, const Vector3& maxs, const Matrix4x4& view_matrix) {
        // Calculate the 8 corners of the bounding box
        Vector3 corners[8] = {
            Vector3(mins.x, mins.y, mins.z),
            Vector3(mins.x, mins.y, maxs.z),
            Vector3(mins.x, maxs.y, mins.z),
            Vector3(mins.x, maxs.y, maxs.z),
            Vector3(maxs.x, mins.y, mins.z),
            Vector3(maxs.x, mins.y, maxs.z),
            Vector3(maxs.x, maxs.y, mins.z),
            Vector3(maxs.x, maxs.y, maxs.z)
        };

        Vector2 screen_points[8];
        int valid_points = 0;
        
        for (int i = 0; i < 8; i++) {
            Vector4 clip_coords = {
                corners[i].x * view_matrix.m[0][0] + corners[i].y * view_matrix.m[0][1] + corners[i].z * view_matrix.m[0][2] + view_matrix.m[0][3],
                corners[i].x * view_matrix.m[1][0] + corners[i].y * view_matrix.m[1][1] + corners[i].z * view_matrix.m[1][2] + view_matrix.m[1][3],
                corners[i].x * view_matrix.m[2][0] + corners[i].y * view_matrix.m[2][1] + corners[i].z * view_matrix.m[2][2] + view_matrix.m[2][3],
                corners[i].x * view_matrix.m[3][0] + corners[i].y * view_matrix.m[3][1] + corners[i].z * view_matrix.m[3][2] + view_matrix.m[3][3]
            };
            
            if (clip_coords.w > 0.001f) {
                Vector3 ndc = {
                    clip_coords.x / clip_coords.w,
                    clip_coords.y / clip_coords.w,
                    clip_coords.z / clip_coords.w
                };
                
                // Get screen size (assuming 1920x1080 for now, should be dynamic)
                ImGuiIO& io = ImGui::GetIO();
                screen_points[valid_points].x = (ndc.x + 1.0f) * 0.5f * io.DisplaySize.x;
                screen_points[valid_points].y = (1.0f - ndc.y) * 0.5f * io.DisplaySize.y;
                valid_points++;
            }
        }

        if (valid_points == 0) {
            return ImRect(0, 0, 0, 0);
        }

        // Find min/max of valid screen points
        float min_x = screen_points[0].x, max_x = screen_points[0].x;
        float min_y = screen_points[0].y, max_y = screen_points[0].y;
        
        for (int i = 1; i < valid_points; i++) {
            min_x = std::min(min_x, screen_points[i].x);
            max_x = std::max(max_x, screen_points[i].x);
            min_y = std::min(min_y, screen_points[i].y);
            max_y = std::max(max_y, screen_points[i].y);
        }

        return ImRect(min_x, min_y, max_x, max_y);
    }
}

// Drawing helper functions
inline void DrawString(const Vector2& pos, const char* text, const ImColor& color = ImColor(255, 255, 255, 255)) {
    Drawing::GetDrawList()->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + ImGui::CalcTextSize(text).x, pos.y + ImGui::CalcTextSize(text).y), ImColor(0, 0, 0, 180));
    Drawing::GetDrawList()->AddText(ImVec2(pos.x, pos.y), color, text);
}

inline void DrawLine(const Vector2& start, const Vector2& end, float thickness, const ImColor& color) {
    Drawing::GetDrawList()->AddLine(ImVec2(start.x, start.y), ImVec2(end.x, end.y), color, thickness);
}

inline void DrawRect(const Vector2& min, const Vector2& max, const ImColor& color, float thickness, bool corners_only = false, const Vector2& corner_size = Vector2(0, 0)) {
    if (corners_only && corner_size.x > 0 && corner_size.y > 0) {
        // Draw corner lines
        float corner_w = corner_size.x;
        float corner_h = corner_size.y;
        
        // Top-left corner
        Drawing::GetDrawList()->AddLine(ImVec2(min.x, min.y), ImVec2(min.x + corner_w, min.y), color, thickness);
        Drawing::GetDrawList()->AddLine(ImVec2(min.x, min.y), ImVec2(min.x, min.y + corner_h), color, thickness);
        
        // Top-right corner
        Drawing::GetDrawList()->AddLine(ImVec2(max.x - corner_w, min.y), ImVec2(max.x, min.y), color, thickness);
        Drawing::GetDrawList()->AddLine(ImVec2(max.x, min.y), ImVec2(max.x, min.y + corner_h), color, thickness);
        
        // Bottom-left corner
        Drawing::GetDrawList()->AddLine(ImVec2(min.x, max.y - corner_h), ImVec2(min.x, max.y), color, thickness);
        Drawing::GetDrawList()->AddLine(ImVec2(min.x, max.y), ImVec2(min.x + corner_w, max.y), color, thickness);
        
        // Bottom-right corner
        Drawing::GetDrawList()->AddLine(ImVec2(max.x, max.y - corner_h), ImVec2(max.x, max.y), color, thickness);
        Drawing::GetDrawList()->AddLine(ImVec2(max.x - corner_w, max.y), ImVec2(max.x, max.y), color, thickness);
    } else {
        Drawing::GetDrawList()->AddRect(ImVec2(min.x, min.y), ImVec2(max.x, max.y), color, 0.0f, 0, thickness);
    }
}

inline void DrawFilledRect(const Vector2& min, const Vector2& max, const ImColor& color) {
    Drawing::GetDrawList()->AddRectFilled(ImVec2(min.x, min.y), ImVec2(max.x, max.y), color);
}

// Enhanced DrawRect with shadow and fill support
inline void DrawRectWithOptions(
    const Vector2& min, 
    const Vector2& max, 
    const ImColor& color, 
    float thickness, 
    bool corners_only = false, 
    const Vector2& corner_size = Vector2(0, 0),
    bool draw_shadow = false,
    float shadow_offset = 1.0f,
    const ImColor& shadow_color = ImColor(0, 0, 0, 100),
    bool filled = false,
    const ImColor& fill_color = ImColor(255, 255, 255, 50)
) {
    // Draw shadow first (behind everything)
    if (draw_shadow) {
        Vector2 shadow_min = Vector2(min.x + shadow_offset, min.y + shadow_offset);
        Vector2 shadow_max = Vector2(max.x + shadow_offset, max.y + shadow_offset);
        
        if (filled) {
            Drawing::GetDrawList()->AddRectFilled(
                ImVec2(shadow_min.x, shadow_min.y), 
                ImVec2(shadow_max.x, shadow_max.y), 
                shadow_color
            );
        } else if (corners_only && corner_size.x > 0 && corner_size.y > 0) {
            // Draw corner shadow lines
            float corner_w = corner_size.x;
            float corner_h = corner_size.y;
            
            // Top-left corner shadow
            Drawing::GetDrawList()->AddLine(ImVec2(shadow_min.x, shadow_min.y), ImVec2(shadow_min.x + corner_w, shadow_min.y), shadow_color, thickness);
            Drawing::GetDrawList()->AddLine(ImVec2(shadow_min.x, shadow_min.y), ImVec2(shadow_min.x, shadow_min.y + corner_h), shadow_color, thickness);
            
            // Top-right corner shadow
            Drawing::GetDrawList()->AddLine(ImVec2(shadow_max.x - corner_w, shadow_min.y), ImVec2(shadow_max.x, shadow_min.y), shadow_color, thickness);
            Drawing::GetDrawList()->AddLine(ImVec2(shadow_max.x, shadow_min.y), ImVec2(shadow_max.x, shadow_min.y + corner_h), shadow_color, thickness);
            
            // Bottom-left corner shadow
            Drawing::GetDrawList()->AddLine(ImVec2(shadow_min.x, shadow_max.y - corner_h), ImVec2(shadow_min.x, shadow_max.y), shadow_color, thickness);
            Drawing::GetDrawList()->AddLine(ImVec2(shadow_min.x, shadow_max.y), ImVec2(shadow_min.x + corner_w, shadow_max.y), shadow_color, thickness);
            
            // Bottom-right corner shadow
            Drawing::GetDrawList()->AddLine(ImVec2(shadow_max.x, shadow_max.y - corner_h), ImVec2(shadow_max.x, shadow_max.y), shadow_color, thickness);
            Drawing::GetDrawList()->AddLine(ImVec2(shadow_max.x - corner_w, shadow_max.y), ImVec2(shadow_max.x, shadow_max.y), shadow_color, thickness);
        } else {
            Drawing::GetDrawList()->AddRect(ImVec2(shadow_min.x, shadow_min.y), ImVec2(shadow_max.x, shadow_max.y), shadow_color, 0.0f, 0, thickness);
        }
    }
    
    // Draw filled background
    if (filled) {
        Drawing::GetDrawList()->AddRectFilled(ImVec2(min.x, min.y), ImVec2(max.x, max.y), fill_color);
    }
    
    // Draw the main outline
    if (corners_only && corner_size.x > 0 && corner_size.y > 0) {
        // Draw corner lines
        float corner_w = corner_size.x;
        float corner_h = corner_size.y;
        
        // Top-left corner
        Drawing::GetDrawList()->AddLine(ImVec2(min.x, min.y), ImVec2(min.x + corner_w, min.y), color, thickness);
        Drawing::GetDrawList()->AddLine(ImVec2(min.x, min.y), ImVec2(min.x, min.y + corner_h), color, thickness);
        
        // Top-right corner
        Drawing::GetDrawList()->AddLine(ImVec2(max.x - corner_w, min.y), ImVec2(max.x, min.y), color, thickness);
        Drawing::GetDrawList()->AddLine(ImVec2(max.x, min.y), ImVec2(max.x, min.y + corner_h), color, thickness);
        
        // Bottom-left corner
        Drawing::GetDrawList()->AddLine(ImVec2(min.x, max.y - corner_h), ImVec2(min.x, max.y), color, thickness);
        Drawing::GetDrawList()->AddLine(ImVec2(min.x, max.y), ImVec2(min.x + corner_w, max.y), color, thickness);
        
        // Bottom-right corner
        Drawing::GetDrawList()->AddLine(ImVec2(max.x, max.y - corner_h), ImVec2(max.x, max.y), color, thickness);
        Drawing::GetDrawList()->AddLine(ImVec2(max.x - corner_w, max.y), ImVec2(max.x, max.y), color, thickness);
    } else {
        Drawing::GetDrawList()->AddRect(ImVec2(min.x, min.y), ImVec2(max.x, max.y), color, 0.0f, 0, thickness);
    }
} 