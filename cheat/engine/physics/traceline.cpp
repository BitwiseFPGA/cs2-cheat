#include <engine/physics/traceline.hpp>
#include <logger/logger.hpp>
#include <algorithm>
#include <cmath>

TracelineManager::TracelineManager(WorldCache* world_cache)
    : m_world_cache(world_cache)
    , m_initialized(false)
{
}

TracelineManager::~TracelineManager() {
    if (m_initialized) {
        shutdown();
    }
    logger::info("TracelineManager destructor called");
}

bool TracelineManager::initialize() {
    logger::log_step("Traceline Manager Init", "Setting up traceline system");
    
    try {
        if (!m_world_cache) {
            logger::log_failure("Traceline Manager", "WorldCache is null");
            return false;
        }
        
        m_triangles.clear();
        m_initialized = true;
        
        rebuild_spatial_optimization();

        return true;
        
    } catch (const std::exception& e) {
        logger::log_failure("Traceline Manager", e.what());
        return false;
    }
}

void TracelineManager::shutdown() {
    logger::info("Shutting down TracelineManager");
    
    m_triangles.clear();
    m_spatial_grid = SpatialGrid();
    m_initialized = false;
    
    logger::info("TracelineManager shutdown completed");
}

void TracelineManager::rebuild_spatial_optimization() {
    if (!m_initialized || !m_world_cache) {
        return;
    }
    
    logger::debug("Rebuilding spatial optimization for traceline");
    
    m_triangles = m_world_cache->get_triangles();
    
    build_spatial_grid();
    
    logger::debug("Spatial optimization rebuilt with " + std::to_string(m_triangles.size()) + " triangles");
}

TraceResult TracelineManager::trace_line(const Vector3& start, const Vector3& end, bool ignore_glass) const {
    TraceResult result;
    
    if (!m_initialized || m_triangles.empty()) {
        return result;
    }
    
    Vector3 direction = (end - start).normalized();
    float max_distance = (end - start).length();
    
    float closest_distance = max_distance;
    bool found_hit = false;
    
    std::vector<int> cells_to_check = get_cells_along_ray(start, end);
    
    for (int cell_index : cells_to_check) {
        if (cell_index < 0 || cell_index >= static_cast<int>(m_spatial_grid.cells.size())) {
            continue;
        }
        
        const auto& cell = m_spatial_grid.cells[cell_index];
        
        for (uint32_t triangle_index : cell.triangle_indices) {
            if (triangle_index >= m_triangles.size()) {
                continue;
            }
            
            const Triangle& triangle = m_triangles[triangle_index];
            
            float distance;
            Vector3 hit_point;
            
            if (ray_triangle_intersect(start, direction, triangle, distance, hit_point)) {
                if (distance < closest_distance && distance > 0.001f) { 
                    closest_distance = distance;
                    result.hit = true;
                    result.hit_point = hit_point;
                    result.hit_normal = triangle.normal;
                    result.distance = distance;
                    result.triangle_id = triangle.id;
                    found_hit = true;
                }
            }
        }
    }
    
    return result;
}

bool TracelineManager::is_visible(const Vector3& start, const Vector3& end) const {
    TraceResult result = trace_line(start, end);
    return !result.hit;
}

bool TracelineManager::ray_triangle_intersect(const Vector3& ray_origin, const Vector3& ray_direction, 
                                             const Triangle& triangle, float& distance, Vector3& hit_point) const {
    const float EPSILON = 0.0000001f;
    
    Vector3 edge1 = triangle.vertices[1] - triangle.vertices[0];
    Vector3 edge2 = triangle.vertices[2] - triangle.vertices[0];
    
    Vector3 h = ray_direction.cross(edge2);
    float a = edge1.dot(h);
    
    if (a > -EPSILON && a < EPSILON) {
        return false;
    }
    
    float f = 1.0f / a;
    Vector3 s = ray_origin - triangle.vertices[0];
    float u = f * s.dot(h);
    
    if (u < 0.0f || u > 1.0f) {
        return false;
    }
    
    Vector3 q = s.cross(edge1);
    float v = f * ray_direction.dot(q);
    
    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }
    
    float t = f * edge2.dot(q);
    
    if (t > EPSILON) {
        distance = t;
        hit_point = ray_origin + ray_direction * t;
        return true;
    }
    
    return false;
}

void TracelineManager::build_spatial_grid() {
    if (m_triangles.empty()) {
        return;
    }
    
    Vector3 world_min(FLT_MAX, FLT_MAX, FLT_MAX);
    Vector3 world_max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    
    for (const auto& triangle : m_triangles) {
        for (int i = 0; i < 3; ++i) {
            const Vector3& vertex = triangle.vertices[i];
            world_min.x = std::min(world_min.x, vertex.x);
            world_min.y = std::min(world_min.y, vertex.y);
            world_min.z = std::min(world_min.z, vertex.z);
            world_max.x = std::max(world_max.x, vertex.x);
            world_max.y = std::max(world_max.y, vertex.y);
            world_max.z = std::max(world_max.z, vertex.z);
        }
    }
    
    const int GRID_RESOLUTION = 64;
    m_spatial_grid.grid_min = world_min;
    m_spatial_grid.grid_max = world_max;
    m_spatial_grid.grid_width = GRID_RESOLUTION;
    m_spatial_grid.grid_height = GRID_RESOLUTION;
    m_spatial_grid.grid_depth = GRID_RESOLUTION;
    
    Vector3 world_size = world_max - world_min;
    m_spatial_grid.cell_size = Vector3(
        world_size.x / GRID_RESOLUTION,
        world_size.y / GRID_RESOLUTION,
        world_size.z / GRID_RESOLUTION
    );
    
    int total_cells = GRID_RESOLUTION * GRID_RESOLUTION * GRID_RESOLUTION;
    m_spatial_grid.cells.resize(total_cells);
    
    for (uint32_t i = 0; i < m_triangles.size(); ++i) {
        const Triangle& triangle = m_triangles[i];
        
        Vector3 tri_min = triangle.vertices[0];
        Vector3 tri_max = triangle.vertices[0];
        
        for (int j = 1; j < 3; ++j) {
            tri_min.x = std::min(tri_min.x, triangle.vertices[j].x);
            tri_min.y = std::min(tri_min.y, triangle.vertices[j].y);
            tri_min.z = std::min(tri_min.z, triangle.vertices[j].z);
            tri_max.x = std::max(tri_max.x, triangle.vertices[j].x);
            tri_max.y = std::max(tri_max.y, triangle.vertices[j].y);
            tri_max.z = std::max(tri_max.z, triangle.vertices[j].z);
        }
        
        int min_x = std::max(0, static_cast<int>((tri_min.x - world_min.x) / m_spatial_grid.cell_size.x));
        int max_x = std::min(GRID_RESOLUTION - 1, static_cast<int>((tri_max.x - world_min.x) / m_spatial_grid.cell_size.x));
        int min_y = std::max(0, static_cast<int>((tri_min.y - world_min.y) / m_spatial_grid.cell_size.y));
        int max_y = std::min(GRID_RESOLUTION - 1, static_cast<int>((tri_max.y - world_min.y) / m_spatial_grid.cell_size.y));
        int min_z = std::max(0, static_cast<int>((tri_min.z - world_min.z) / m_spatial_grid.cell_size.z));
        int max_z = std::min(GRID_RESOLUTION - 1, static_cast<int>((tri_max.z - world_min.z) / m_spatial_grid.cell_size.z));
        
        for (int x = min_x; x <= max_x; ++x) {
            for (int y = min_y; y <= max_y; ++y) {
                for (int z = min_z; z <= max_z; ++z) {
                    int cell_index = z * GRID_RESOLUTION * GRID_RESOLUTION + y * GRID_RESOLUTION + x;
                    m_spatial_grid.cells[cell_index].triangle_indices.push_back(i);
                }
            }
        }
    }
}

int TracelineManager::get_grid_cell_index(const Vector3& position) const {
    Vector3 relative_pos = position - m_spatial_grid.grid_min;
    
    int x = static_cast<int>(relative_pos.x / m_spatial_grid.cell_size.x);
    int y = static_cast<int>(relative_pos.y / m_spatial_grid.cell_size.y);
    int z = static_cast<int>(relative_pos.z / m_spatial_grid.cell_size.z);
    
    x = std::max(0, std::min(x, m_spatial_grid.grid_width - 1));
    y = std::max(0, std::min(y, m_spatial_grid.grid_height - 1));
    z = std::max(0, std::min(z, m_spatial_grid.grid_depth - 1));
    
    return z * m_spatial_grid.grid_width * m_spatial_grid.grid_height + y * m_spatial_grid.grid_width + x;
}

std::vector<int> TracelineManager::get_cells_along_ray(const Vector3& start, const Vector3& end) const {
    std::vector<int> cells;
    
    Vector3 direction = end - start;
    float length = direction.length();
    direction = direction.normalized();
    
    const float STEP_SIZE = std::min({m_spatial_grid.cell_size.x, m_spatial_grid.cell_size.y, m_spatial_grid.cell_size.z}) * 0.5f;
    
    for (float t = 0; t <= length; t += STEP_SIZE) {
        Vector3 sample_point = start + direction * t;
        int cell_index = get_grid_cell_index(sample_point);
        
        if (cells.empty() || cells.back() != cell_index) {
            cells.push_back(cell_index);
        }
    }
    
    return cells;
}

bool TracelineManager::point_in_triangle(const Vector3& point, const Triangle& triangle) const {
    Vector3 v0 = triangle.vertices[2] - triangle.vertices[0];
    Vector3 v1 = triangle.vertices[1] - triangle.vertices[0];
    Vector3 v2 = point - triangle.vertices[0];
    
    float dot00 = v0.dot(v0);
    float dot01 = v0.dot(v1);
    float dot02 = v0.dot(v2);
    float dot11 = v1.dot(v1);
    float dot12 = v1.dot(v2);
    
    float inv_denom = 1 / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * inv_denom;
    float v = (dot00 * dot12 - dot01 * dot02) * inv_denom;
    
    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

float TracelineManager::calculate_triangle_distance(const Vector3& point, const Triangle& triangle) const {
    Vector3 to_point = point - triangle.vertices[0];
    return std::abs(to_point.dot(triangle.normal));
} 