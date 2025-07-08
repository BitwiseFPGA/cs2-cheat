#pragma once
#include <logger/logger.hpp>
#include <engine/sdk/math/vector.hpp>
#include <engine/sdk/math/matrix.hpp>
#include <engine/sdk/math/triangle.hpp>
#include <access/access.hpp>
#include <engine/physics/traceline.hpp>

#include <vector>
#include <memory>
#include <chrono>
#include <unordered_set>

class AccessManager;
class TracelineManager;
class Engine;

/*

std::vector<int> current_nodes;
std::vector<int> next_nodes;
std::unordered_set<int> visited;

current_nodes.push_back(root_index);

while (!current_nodes.empty()) {
    next_nodes.clear();

    std::vector<physx::TreeNode> nodes(current_nodes.size());
    for (size_t i = 0; i < current_nodes.size(); i++) {
        int node_index = current_nodes[i];
        if (node_index < 0 || node_index >= array_size || visited.count(node_index)) {
            continue;
        }
        
        uintptr_t node_ptr = nodes_array + (node_index * sizeof(physx::TreeNode));
        m_access_manager->add_scatter_read(m_scatter_handle, node_ptr, &nodes[i], sizeof(physx::TreeNode));
    }
    
    if (!m_access_manager->scatter_read(m_scatter_handle)) {
        logger::debug("Failed to read node batch for tree " + std::to_string(tree_index));
        break;
    }

    std::vector<uintptr_t> geometry_addresses;
    for (size_t i = 0; i < current_nodes.size(); i++) {
        int node_index = current_nodes[i];
        if (node_index < 0 || node_index >= array_size || visited.count(node_index)) {
            continue;
        }
        
        visited.insert(node_index);
        const physx::TreeNode& node = nodes[i];

        if (node.child_or_leaf == -1) {
            if (node.geometry_ptr) {
                geometry_addresses.push_back(node.geometry_ptr);
            }
        } else {
            next_nodes.push_back(node.child_or_leaf);
            if (node.sibling_index >= 0 && node.sibling_index < array_size) {
                next_nodes.push_back(node.sibling_index);
            }
        }
    }

    if (!geometry_addresses.empty()) {
        std::vector<physx::NodeGeometry> geometries(geometry_addresses.size());
        for (size_t i = 0; i < geometry_addresses.size(); i++) {
            m_access_manager->add_scatter_read(m_scatter_handle, geometry_addresses[i], &geometries[i], sizeof(physx::NodeGeometry));
        }
        
        if (m_access_manager->scatter_read(m_scatter_handle)) {
            for (const auto& geometry : geometries) {
                process_geometry_convex_hulls(geometry);
                process_geometry_triangle_meshes(geometry);
            }
        }
    }

    current_nodes = std::move(next_nodes);
}

*/

namespace physx {
	constexpr uintptr_t trace_mng = 0x182FB20;
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

    struct SpatialTree {
        uintptr_t base;
        int root_index;
        uintptr_t nodes_array;
        int array_size;

        std::vector<TreeNode> nodes;
    };
}

class WorldCache {
public:
    WorldCache(AccessManager* access_manager, TracelineManager* traceline_manager, Engine* engine);
    ~WorldCache();
    
    bool initialize();
    void shutdown();
    
    void update();
    void clear();
    
    const std::vector<Triangle>& get_triangles() const { return m_triangles; }
    
private:
    AccessManager* m_access_manager;
    TracelineManager* m_traceline_manager;
    Engine* m_engine;
    ScatterHandle m_scatter_handle;
    bool m_initialized;

    std::uintptr_t m_client_dll_base;
    std::uintptr_t m_prev_physx_world;
    
    std::chrono::milliseconds m_last_update;
    
    std::vector<Triangle> m_triangles;
    
    void load_world_triangles();
};
