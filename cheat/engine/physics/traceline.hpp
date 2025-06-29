#pragma once
#include <engine/sdk/math/vector.hpp>
#include <engine/sdk/math/triangle.hpp>
#include <memory>
#include <vector>

struct TraceResult {
    bool hit;
    Vector3 hit_point;
    Vector3 hit_normal;
    float distance;
    uint32_t triangle_id;
    
    TraceResult() : hit(false), distance(0.0f), triangle_id(0) {}
};

struct SpatialGrid {
    struct Cell {
        std::vector<uint32_t> triangle_indices;
    };
    
    std::vector<Cell> cells;
    Vector3 grid_min;
    Vector3 grid_max;
    Vector3 cell_size;
    int grid_width, grid_height, grid_depth;
    
    SpatialGrid() : grid_width(0), grid_height(0), grid_depth(0) {}
};

class TracelineManager {
public:
    TracelineManager();
    ~TracelineManager();
    
    bool initialize();
    void shutdown();
    
    void rebuild_spatial_optimization(std::vector<Triangle>& new_triangles);
    
    TraceResult trace_line(const Vector3& start, const Vector3& end, bool ignore_glass = false) const;
    
    bool is_visible(const Vector3& start, const Vector3& end) const;
    
    bool ray_triangle_intersect(const Vector3& ray_origin, const Vector3& ray_direction, 
                               const Triangle& triangle, float& distance, Vector3& hit_point) const;
        
private:
    bool m_initialized;
    
    std::vector<Triangle> m_triangles;
    
    SpatialGrid m_spatial_grid;
    
    void build_spatial_grid();
    int get_grid_cell_index(const Vector3& position) const;
    std::vector<int> get_cells_along_ray(const Vector3& start, const Vector3& end) const;
    
    bool point_in_triangle(const Vector3& point, const Triangle& triangle) const;
    float calculate_triangle_distance(const Vector3& point, const Triangle& triangle) const;
}; 