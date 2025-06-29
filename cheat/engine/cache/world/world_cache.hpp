#pragma once
#include <logger/logger.hpp>
#include <engine/sdk/math/vector.hpp>
#include <engine/sdk/math/matrix.hpp>
#include <engine/sdk/math/triangle.hpp>
#include <vector>
#include <memory>
#include <chrono>
#include <unordered_set>
#include <access/access.hpp>
#include <engine/physics/traceline.hpp>

class AccessManager;
class TracelineManager;

namespace physx {
	constexpr uintptr_t trace_mng = 0x182DB00;
	constexpr uintptr_t spatial_trees_base = 0x20F8;

    struct TreeNode {
        Vector3 bbox_min;        // 0x0
        int child_or_leaf;       // 0xC
        Vector3 bbox_max;        // 0x10  
        int sibling_index;       // 0x1C
        int padding1;            // 0x20
        int padding2;            // 0x24
        uintptr_t geometry_ptr;  // 0x28
    };

    struct NodeGeometry {
        uint8_t padding[0xC0];   // 0x0
        uintptr_t shape_ptr;     // 0xC0
        uintptr_t mesh_data;     // 0xC8
    };

    struct ConvexHull {
        uint8_t padding[0x90];   // 0x0
        uintptr_t vertices_ptr;  // 0x90
        int vertex_count;        // 0x98
    };

    struct TriangleMesh {
        uint8_t padding[0x38];   // 0x0
        uintptr_t vertices_ptr;  // 0x38
        int vertex_count;        // 0x40
        uint8_t padding2[0x8];   // 0x48
        uintptr_t indices_ptr;   // 0x50
        int triangle_count;      // 0x58
    };
}

class WorldCache {
public:
    WorldCache(AccessManager* access_manager, TracelineManager* traceline_manager);
    ~WorldCache();
    
    bool initialize();
    void shutdown();
    
    void update();
    void clear();
    
    const std::vector<Triangle>& get_triangles() const { return m_triangles; }
    
private:
    AccessManager* m_access_manager;
    TracelineManager* m_traceline_manager;
    ScatterHandle m_scatter_handle;
    bool m_initialized;

    std::uintptr_t prev_physx_world;
    
    std::chrono::milliseconds m_last_update;
    
    std::vector<Triangle> m_triangles;
    
    void load_world_triangles();
    void process_geometry_convex_hulls(const physx::NodeGeometry& geometry);
    void process_geometry_triangle_meshes(const physx::NodeGeometry& geometry);
};
