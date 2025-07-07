#pragma once
#include <engine/engine.hpp>
#include <engine/sdk/math/vector.hpp>
#include <engine/sdk/math/matrix.hpp>
#include <engine/sdk/types/player.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <vector>
#include <utility>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

class Engine;

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
    inline void DrawString(Renderer* renderer, const Vector2& pos, const char* text, const ImColor& color = ImColor(255, 255, 255, 255)) {
        renderer->get_draw_list()->AddText(ImVec2(pos.x, pos.y), color, text);
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

    inline void DrawSphere(Renderer* renderer, const Vector3& world_center, const ImColor& color, float radius, Engine* engine, int segments = 16) {
        if (!engine) return;
        
        // Generate sphere vertices
        std::vector<Vector3> vertices;
        std::vector<std::pair<int, int>> edges;
        
        // Generate vertices for a sphere using spherical coordinates
        for (int lat = 0; lat <= segments; lat++) {
            float theta = M_PI * lat / segments; // 0 to PI
            for (int lon = 0; lon <= segments; lon++) {
                float phi = 2.0f * M_PI * lon / segments; // 0 to 2*PI
                
                Vector3 vertex = {
                    world_center.x + radius * sin(theta) * cos(phi),
                    world_center.y + radius * sin(theta) * sin(phi),
                    world_center.z + radius * cos(theta)
                };
                vertices.push_back(vertex);
            }
        }
        
        // Generate edges for wireframe (longitude and latitude lines)
        for (int lat = 0; lat < segments; lat++) {
            for (int lon = 0; lon < segments; lon++) {
                int current = lat * (segments + 1) + lon;
                int next_lat = (lat + 1) * (segments + 1) + lon;
                int next_lon = lat * (segments + 1) + (lon + 1);
                
                // Latitude lines
                if (lat < segments) {
                    edges.push_back({current, next_lat});
                }
                // Longitude lines
                if (lon < segments) {
                    edges.push_back({current, next_lon});
                }
            }
        }
        
        // Project vertices to screen and draw edges
        std::vector<Vector2> screen_vertices(vertices.size());
        std::vector<bool> vertex_visible(vertices.size(), false);
        
        for (size_t i = 0; i < vertices.size(); i++) {
            vertex_visible[i] = engine->world_to_screen(vertices[i], screen_vertices[i]);
        }
        
        // Draw wireframe edges
        for (const auto& edge : edges) {
            if (vertex_visible[edge.first] && vertex_visible[edge.second]) {
                renderer->get_draw_list()->AddLine(
                    ImVec2(screen_vertices[edge.first].x, screen_vertices[edge.first].y),
                    ImVec2(screen_vertices[edge.second].x, screen_vertices[edge.second].y),
                    color,
                    1.0f
                );
            }
        }
    }

    inline void DrawVoxelCube(Renderer* renderer, const Vector3& center, float size, float density, Engine* engine, 
                              float min_density_threshold = 0.01f, 
                              float max_opacity = 0.8f, 
                              float density_multiplier = 1.0f,
                              bool use_gradient_colors = false,
                              const ImColor& low_density_color = ImColor(1.0f, 1.0f, 1.0f, 1.0f),
                              const ImColor& high_density_color = ImColor(0.8f, 0.8f, 0.8f, 1.0f),
                              bool show_edges = true,
                              float edge_thickness = 1.0f,
                              const ImColor& edge_color = ImColor(0.7f, 0.7f, 0.7f, 1.0f)) {
        if (!engine) return;
        
        // Skip voxel if density is below threshold
        if (density < min_density_threshold) return;

        const float HALF_SIZE = size * 0.5f;
        
        // Define the 8 corners of the cube
        Vector3 corners[8] = {
            {center.x - HALF_SIZE, center.y - HALF_SIZE, center.z - HALF_SIZE}, // Front bottom left (0)
            {center.x + HALF_SIZE, center.y - HALF_SIZE, center.z - HALF_SIZE}, // Front bottom right (1)
            {center.x + HALF_SIZE, center.y + HALF_SIZE, center.z - HALF_SIZE}, // Back bottom right (2)
            {center.x - HALF_SIZE, center.y + HALF_SIZE, center.z - HALF_SIZE}, // Back bottom left (3)
            {center.x - HALF_SIZE, center.y - HALF_SIZE, center.z + HALF_SIZE}, // Front top left (4)
            {center.x + HALF_SIZE, center.y - HALF_SIZE, center.z + HALF_SIZE}, // Front top right (5)
            {center.x + HALF_SIZE, center.y + HALF_SIZE, center.z + HALF_SIZE}, // Back top right (6)
            {center.x - HALF_SIZE, center.y + HALF_SIZE, center.z + HALF_SIZE}  // Back top left (7)
        };

        // Project corners to screen space
        Vector2 screen_corners[8];
        bool all_corners_visible = true;
        float depths[8];

        for (int i = 0; i < 8; i++) {
            if (!engine->world_to_screen(corners[i], screen_corners[i])) {
                all_corners_visible = false;
                break;
            }
            // Store Z depth for sorting
            depths[i] = corners[i].z;
        }

        if (!all_corners_visible) return;

        // Define cube faces with vertex indices and calculate average depth
        struct Face {
            int indices[4];  // Vertex indices
            float avg_depth; // Average depth for sorting
            int face_idx;    // Original face index for stable sorting
        };

        Face faces[6] = {
            {{0, 1, 2, 3}, 0.0f, 0}, // Front
            {{5, 4, 7, 6}, 0.0f, 1}, // Back
            {{4, 0, 3, 7}, 0.0f, 2}, // Left
            {{1, 5, 6, 2}, 0.0f, 3}, // Right
            {{4, 5, 1, 0}, 0.0f, 4}, // Bottom
            {{3, 2, 6, 7}, 0.0f, 5}  // Top
        };

        // Calculate average depth for each face
        for (auto& face : faces) {
            face.avg_depth = 0.0f;
            for (int i = 0; i < 4; i++) {
                face.avg_depth += depths[face.indices[i]];
            }
            face.avg_depth /= 4.0f;
        }

        // Sort faces by depth (back to front)
        std::sort(faces, faces + 6, [](const Face& a, const Face& b) {
            return a.avg_depth > b.avg_depth;
        });

        // Calculate colors based on density and settings
        float alpha_base = std::min(density * density_multiplier * max_opacity, max_opacity);
        
        ImColor face_color;
        if (use_gradient_colors) {
            // Interpolate between low and high density colors based on density
            float t = std::min(density * density_multiplier, 1.0f);
            face_color = ImColor(
                low_density_color.Value.x * (1.0f - t) + high_density_color.Value.x * t,
                low_density_color.Value.y * (1.0f - t) + high_density_color.Value.y * t,
                low_density_color.Value.z * (1.0f - t) + high_density_color.Value.z * t,
                alpha_base
            );
        } else {
            face_color = ImColor(
                low_density_color.Value.x,
                low_density_color.Value.y,
                low_density_color.Value.z,
                alpha_base
            );
        }
        
        ImColor final_edge_color(
            edge_color.Value.x,
            edge_color.Value.y,
            edge_color.Value.z,
            alpha_base
        );

        // Draw faces from back to front
        for (const auto& face : faces) {
            // Draw filled face
            renderer->get_draw_list()->AddQuadFilled(
                ImVec2(screen_corners[face.indices[0]].x, screen_corners[face.indices[0]].y),
                ImVec2(screen_corners[face.indices[1]].x, screen_corners[face.indices[1]].y),
                ImVec2(screen_corners[face.indices[2]].x, screen_corners[face.indices[2]].y),
                ImVec2(screen_corners[face.indices[3]].x, screen_corners[face.indices[3]].y),
                face_color
            );

            // Draw edges if enabled
            if (show_edges) {
                for (int i = 0; i < 4; i++) {
                    int j = (i + 1) % 4;
                    renderer->get_draw_list()->AddLine(
                        ImVec2(screen_corners[face.indices[i]].x, screen_corners[face.indices[i]].y),
                        ImVec2(screen_corners[face.indices[j]].x, screen_corners[face.indices[j]].y),
                        final_edge_color,
                        edge_thickness
                    );
                }
            }
        }
    }
}

// Drawing helper functions
inline void DrawString(Renderer* renderer, const Vector2& pos, const char* text, const ImColor& color = ImColor(255, 255, 255, 255)) {
    renderer->get_draw_list()->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + ImGui::CalcTextSize(text).x, pos.y + ImGui::CalcTextSize(text).y), ImColor(0, 0, 0, 180));
    renderer->get_draw_list()->AddText(ImVec2(pos.x, pos.y), color, text);
}

inline void DrawLine(Renderer* renderer, const Vector2& start, const Vector2& end, float thickness, const ImColor& color) {
    renderer->get_draw_list()->AddLine(ImVec2(start.x, start.y), ImVec2(end.x, end.y), color, thickness);
}

inline void DrawRect(Renderer* renderer, const Vector2& min, const Vector2& max, const ImColor& color, float thickness, bool corners_only = false, const Vector2& corner_size = Vector2(0, 0)) {
    if (corners_only && corner_size.x > 0 && corner_size.y > 0) {
        // Draw corner lines
        float corner_w = corner_size.x;
        float corner_h = corner_size.y;
        
        // Top-left corner
        renderer->get_draw_list()->AddLine(ImVec2(min.x, min.y), ImVec2(min.x + corner_w, min.y), color, thickness);
        renderer->get_draw_list()->AddLine(ImVec2(min.x, min.y), ImVec2(min.x, min.y + corner_h), color, thickness);
        
        // Top-right corner
        renderer->get_draw_list()->AddLine(ImVec2(max.x - corner_w, min.y), ImVec2(max.x, min.y), color, thickness);
        renderer->get_draw_list()->AddLine(ImVec2(max.x, min.y), ImVec2(max.x, min.y + corner_h), color, thickness);
        
        // Bottom-left corner
        renderer->get_draw_list()->AddLine(ImVec2(min.x, max.y - corner_h), ImVec2(min.x, max.y), color, thickness);
        renderer->get_draw_list()->AddLine(ImVec2(min.x, max.y), ImVec2(min.x + corner_w, max.y), color, thickness);
        
        // Bottom-right corner
        renderer->get_draw_list()->AddLine(ImVec2(max.x, max.y - corner_h), ImVec2(max.x, max.y), color, thickness);
        renderer->get_draw_list()->AddLine(ImVec2(max.x - corner_w, max.y), ImVec2(max.x, max.y), color, thickness);
    } else {
        renderer->get_draw_list()->AddRect(ImVec2(min.x, min.y), ImVec2(max.x, max.y), color, 0.0f, 0, thickness);
    }
}

inline void DrawFilledRect(Renderer* renderer, const Vector2& min, const Vector2& max, const ImColor& color) {
    renderer->get_draw_list()->AddRectFilled(ImVec2(min.x, min.y), ImVec2(max.x, max.y), color);
}

// Enhanced DrawRect with shadow and fill support
inline void DrawRectWithOptions(
    Renderer* renderer,
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
            renderer->get_draw_list()->AddRectFilled(
                ImVec2(shadow_min.x, shadow_min.y), 
                ImVec2(shadow_max.x, shadow_max.y), 
                shadow_color
            );
        } else if (corners_only && corner_size.x > 0 && corner_size.y > 0) {
            // Draw corner shadow lines
            float corner_w = corner_size.x;
            float corner_h = corner_size.y;
            
            // Top-left corner shadow
            renderer->get_draw_list()->AddLine(ImVec2(shadow_min.x, shadow_min.y), ImVec2(shadow_min.x + corner_w, shadow_min.y), shadow_color, thickness);
            renderer->get_draw_list()->AddLine(ImVec2(shadow_min.x, shadow_min.y), ImVec2(shadow_min.x, shadow_min.y + corner_h), shadow_color, thickness);
            
            // Top-right corner shadow
            renderer->get_draw_list()->AddLine(ImVec2(shadow_max.x - corner_w, shadow_min.y), ImVec2(shadow_max.x, shadow_min.y), shadow_color, thickness);
            renderer->get_draw_list()->AddLine(ImVec2(shadow_max.x, shadow_min.y), ImVec2(shadow_max.x, shadow_min.y + corner_h), shadow_color, thickness);
            
            // Bottom-left corner shadow
            renderer->get_draw_list()->AddLine(ImVec2(shadow_min.x, shadow_max.y - corner_h), ImVec2(shadow_min.x, shadow_max.y), shadow_color, thickness);
            renderer->get_draw_list()->AddLine(ImVec2(shadow_min.x, shadow_max.y), ImVec2(shadow_min.x + corner_w, shadow_max.y), shadow_color, thickness);
            
            // Bottom-right corner shadow
            renderer->get_draw_list()->AddLine(ImVec2(shadow_max.x, shadow_max.y - corner_h), ImVec2(shadow_max.x, shadow_max.y), shadow_color, thickness);
            renderer->get_draw_list()->AddLine(ImVec2(shadow_max.x - corner_w, shadow_max.y), ImVec2(shadow_max.x, shadow_max.y), shadow_color, thickness);
        } else {
            renderer->get_draw_list()->AddRect(ImVec2(shadow_min.x, shadow_min.y), ImVec2(shadow_max.x, shadow_max.y), shadow_color, 0.0f, 0, thickness);
        }
    }
    
    // Draw filled background
    if (filled) {
        renderer->get_draw_list()->AddRectFilled(ImVec2(min.x, min.y), ImVec2(max.x, max.y), fill_color);
    }
    
    // Draw the main outline
    if (corners_only && corner_size.x > 0 && corner_size.y > 0) {
        // Draw corner lines
        float corner_w = corner_size.x;
        float corner_h = corner_size.y;
        
        // Top-left corner
        renderer->get_draw_list()->AddLine(ImVec2(min.x, min.y), ImVec2(min.x + corner_w, min.y), color, thickness);
        renderer->get_draw_list()->AddLine(ImVec2(min.x, min.y), ImVec2(min.x, min.y + corner_h), color, thickness);
        
        // Top-right corner
        renderer->get_draw_list()->AddLine(ImVec2(max.x - corner_w, min.y), ImVec2(max.x, min.y), color, thickness);
        renderer->get_draw_list()->AddLine(ImVec2(max.x, min.y), ImVec2(max.x, min.y + corner_h), color, thickness);
        
        // Bottom-left corner
        renderer->get_draw_list()->AddLine(ImVec2(min.x, max.y - corner_h), ImVec2(min.x, max.y), color, thickness);
        renderer->get_draw_list()->AddLine(ImVec2(min.x, max.y), ImVec2(min.x + corner_w, max.y), color, thickness);
        
        // Bottom-right corner
        renderer->get_draw_list()->AddLine(ImVec2(max.x, max.y - corner_h), ImVec2(max.x, max.y), color, thickness);
        renderer->get_draw_list()->AddLine(ImVec2(max.x - corner_w, max.y), ImVec2(max.x, max.y), color, thickness);
    } else {
        renderer->get_draw_list()->AddRect(ImVec2(min.x, min.y), ImVec2(max.x, max.y), color, 0.0f, 0, thickness);
    }
} 