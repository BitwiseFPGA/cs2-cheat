#include <engine/cache/world/world_cache.hpp>
#include <access/access.hpp>
#include <logger/logger.hpp>

WorldCache::WorldCache(AccessManager* access_manager, TracelineManager* traceline_manager)
    : m_access_manager(access_manager)
    , m_traceline_manager(traceline_manager)
    , m_initialized(false)
	, m_scatter_handle(nullptr)
    , m_last_update(std::chrono::milliseconds(0))
{
}

WorldCache::~WorldCache() {
    if (m_initialized) {
        shutdown();
    }
    logger::info("WorldCache destructor called");
}

bool WorldCache::initialize() {
    logger::log_step("World Cache Init", "Setting up world data management");
    
    try {
        if (!m_access_manager) {
            logger::log_failure("World Cache", "Memory manager is null");
            return false;
        }

        m_scatter_handle = m_access_manager->create_scatter_handle();
        if (!m_scatter_handle) {
            logger::log_failure("World Cache", "Failed to create scatter handle");
            return false;
        }

        clear();
        m_initialized = true;

        return true;
        
    } catch (const std::exception& e) {
        logger::log_failure("World Cache", e.what());
        return false;
    }
}

void WorldCache::shutdown() {
    logger::info("Shutting down WorldCache");
    if (m_scatter_handle) {
        m_access_manager->close_scatter_handle(m_scatter_handle);
    }
    clear();
    m_initialized = false;
    
    logger::info("WorldCache shutdown completed");
}

void WorldCache::update() {
    if (!m_initialized || !m_access_manager || !m_access_manager->is_attached()) {
        return;
    }
    
    try {
        logger::debug("Updating world cache");
        load_world_triangles();
        
        m_last_update = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
            
    } catch (const std::exception& e) {
        logger::error("WorldCache update failed: " + std::string(e.what()));
    }
}

void WorldCache::clear() {
    logger::debug("Clearing world cache");
    m_triangles.clear();
}

void WorldCache::load_world_triangles() {
    if (!m_access_manager || !m_access_manager->is_attached() || !m_scatter_handle) {
        return;
    }

    try {
        logger::debug("Starting world triangles load with scatter operations");

        // Get client.dll base address
        uintptr_t client_dll_base = m_access_manager->get_module_base("client.dll");
        if (!client_dll_base) {
            logger::error("Failed to get client.dll base address");
            return;
        }

        // Step 1: Read initial PhysX data structures
        uintptr_t tracemng = 0;
        uintptr_t physx_world_ptr = 0;
        uintptr_t physx_world = 0;
        uintptr_t spatial_trees_base = 0;

        // First batch: Read trace manager
        m_access_manager->add_scatter_read(m_scatter_handle, client_dll_base + physx::trace_mng, &tracemng, sizeof(tracemng));
        if (!m_access_manager->scatter_read(m_scatter_handle)) {
            logger::error("Failed to read trace manager");
            return;
        }

        if (!tracemng) {
            logger::error("Invalid trace manager address");
            return;
        }

        // Second batch: Read physx world pointer
        m_access_manager->add_scatter_read(m_scatter_handle, tracemng, &physx_world_ptr, sizeof(physx_world_ptr));
        if (!m_access_manager->scatter_read(m_scatter_handle)) {
            logger::error("Failed to read physx world pointer");
            return;
        }

        if (!physx_world_ptr) {
            logger::error("Invalid physx world pointer");
            return;
        }

        // Third batch: Read physx world and spatial trees base
        m_access_manager->add_scatter_read(m_scatter_handle, physx_world_ptr + 0x30, &physx_world, sizeof(physx_world));
        if (!m_access_manager->scatter_read(m_scatter_handle)) {
            logger::error("Failed to read physx world");
            return;
        }

        if (!physx_world) {
            logger::error("Invalid physx world address");
            return;
        }

        if (prev_physx_world == physx_world) {
            logger::debug("PhysX world has not changed, skipping load");
            return;
		}

		prev_physx_world = physx_world;

        // Fourth batch: Read spatial trees base
        m_access_manager->add_scatter_read(m_scatter_handle, physx_world + physx::spatial_trees_base, &spatial_trees_base, sizeof(spatial_trees_base));
        if (!m_access_manager->scatter_read(m_scatter_handle)) {
            logger::error("Failed to read spatial trees base");
            return;
        }

        if (!spatial_trees_base) {
            logger::error("Invalid spatial trees base address");
            return;
        }

        // Clear previous triangles
        m_triangles.clear();

        // Step 2: Process each spatial tree (3 trees total)
        for (int tree_index = 0; tree_index < 3; tree_index++) {
            uintptr_t spatial_tree = spatial_trees_base + (tree_index * 40);
            
            int root_index = 0;
            uintptr_t nodes_array = 0;
            int array_size = 0;

            // Read tree header info
            m_access_manager->add_scatter_read(m_scatter_handle, spatial_tree, &root_index, sizeof(root_index));
            m_access_manager->add_scatter_read(m_scatter_handle, spatial_tree + 0x10, &nodes_array, sizeof(nodes_array));
            m_access_manager->add_scatter_read(m_scatter_handle, spatial_tree + 0x18, &array_size, sizeof(array_size));
            if (!m_access_manager->scatter_read(m_scatter_handle)) {
                logger::debug("Failed to read tree " + std::to_string(tree_index) + " header, skipping");
                continue;
            }

            if (root_index < 0 || !nodes_array || array_size <= 0) {
                logger::debug("Invalid tree " + std::to_string(tree_index) + " data, skipping");
                continue;
            }

            // Step 3: Traverse nodes using BFS with batched reads
            std::vector<int> current_nodes;
            std::vector<int> next_nodes;
            std::unordered_set<int> visited;
            
            current_nodes.push_back(root_index);

            while (!current_nodes.empty()) {
                next_nodes.clear();

                // Batch read current level nodes
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

                // Process nodes and collect leaf geometry and child nodes
                std::vector<uintptr_t> geometry_addresses;
                for (size_t i = 0; i < current_nodes.size(); i++) {
                    int node_index = current_nodes[i];
                    if (node_index < 0 || node_index >= array_size || visited.count(node_index)) {
                        continue;
                    }
                    
                    visited.insert(node_index);
                    const physx::TreeNode& node = nodes[i];

                    if (node.child_or_leaf == -1) {
                        // Leaf node - collect geometry
                        if (node.geometry_ptr) {
                            geometry_addresses.push_back(node.geometry_ptr);
                        }
                    } else {
                        // Internal node - add children to next level
                        next_nodes.push_back(node.child_or_leaf);
                        if (node.sibling_index >= 0 && node.sibling_index < array_size) {
                            next_nodes.push_back(node.sibling_index);
                        }
                    }
                }

                // Step 4: Batch read geometry data
                if (!geometry_addresses.empty()) {
                    std::vector<physx::NodeGeometry> geometries(geometry_addresses.size());
                    for (size_t i = 0; i < geometry_addresses.size(); i++) {
                        m_access_manager->add_scatter_read(m_scatter_handle, geometry_addresses[i], &geometries[i], sizeof(physx::NodeGeometry));
                    }
                    
                    if (m_access_manager->scatter_read(m_scatter_handle)) {
                        // Process convex hulls and triangle meshes
                        for (const auto& geometry : geometries) {
                            process_geometry_convex_hulls(geometry);
                            process_geometry_triangle_meshes(geometry);
                        }
                    }
                }

                current_nodes = std::move(next_nodes);
            }
        }

        logger::debug("Loaded " + std::to_string(m_triangles.size()) + " triangles from world cache");
        m_traceline_manager->rebuild_spatial_optimization(m_triangles);

    } catch (const std::exception& e) {
        logger::error("Exception in load_world_triangles: " + std::string(e.what()));
    }
}

void WorldCache::process_geometry_convex_hulls(const physx::NodeGeometry& geometry) {
    if (!geometry.shape_ptr) {
        return;
    }

    try {
        physx::ConvexHull hull;
        m_access_manager->add_scatter_read(m_scatter_handle, geometry.shape_ptr, &hull, sizeof(hull));
        if (!m_access_manager->scatter_read(m_scatter_handle)) {
            return;
        }

        if (!hull.vertices_ptr || hull.vertex_count <= 2 || hull.vertex_count >= 10000) {
            return;
        }

        // Read vertices in batches
        std::vector<Vector3> vertices(hull.vertex_count);
        const size_t batch_size = 50; // Read 50 vertices at a time
        
        for (int start = 0; start < hull.vertex_count; start += batch_size) {
            int end = std::min<int>(start + static_cast<int>(batch_size), hull.vertex_count);
            int count = end - start;
            
            for (int i = 0; i < count; i++) {
                uintptr_t vertex_addr = hull.vertices_ptr + ((start + i) * sizeof(Vector3));
                m_access_manager->add_scatter_read(m_scatter_handle, vertex_addr, &vertices[start + i], sizeof(Vector3));
            }
            
            if (!m_access_manager->scatter_read(m_scatter_handle)) {
                return;
            }
        }

        // Create triangles using fan triangulation
        for (int i = 1; i < vertices.size() - 1; i++) {
            m_triangles.emplace_back(vertices[0], vertices[i], vertices[i + 1], static_cast<uint32_t>(m_triangles.size()));
        }

    } catch (const std::exception& e) {
        logger::debug("Exception processing convex hull: " + std::string(e.what()));
    }
}

void WorldCache::process_geometry_triangle_meshes(const physx::NodeGeometry& geometry) {
    if (!geometry.mesh_data) {
        return;
    }

    try {
        physx::TriangleMesh mesh;
        m_access_manager->add_scatter_read(m_scatter_handle, geometry.mesh_data, &mesh, sizeof(mesh));
        if (!m_access_manager->scatter_read(m_scatter_handle)) {
            return;
        }

        if (!mesh.vertices_ptr || mesh.vertex_count <= 0 ||
            !mesh.indices_ptr || mesh.triangle_count <= 0 ||
            mesh.vertex_count >= 100000 || mesh.triangle_count >= 100000) {
            return;
        }

        // Read vertices in batches
        std::vector<Vector3> vertices(mesh.vertex_count);
        const size_t vertex_batch_size = 50;
        
        for (int start = 0; start < mesh.vertex_count; start += vertex_batch_size) {
            int end = std::min<int>(start + static_cast<int>(vertex_batch_size), mesh.vertex_count);
            int count = end - start;
            
            for (int i = 0; i < count; i++) {
                uintptr_t vertex_addr = mesh.vertices_ptr + ((start + i) * sizeof(Vector3));
                m_access_manager->add_scatter_read(m_scatter_handle, vertex_addr, &vertices[start + i], sizeof(Vector3));
            }
            
            if (!m_access_manager->scatter_read(m_scatter_handle)) {
                return;
            }
        }

        // Read triangle indices in batches
        std::vector<int> indices(mesh.triangle_count * 3);
        const size_t index_batch_size = 50;
        
        for (int start = 0; start < mesh.triangle_count; start += index_batch_size) {
            int end = std::min<int>(start + static_cast<int>(index_batch_size), mesh.triangle_count);
            int count = end - start;
            
            for (int i = 0; i < count; i++) {
                int triangle_idx = start + i;
                uintptr_t idx_addr = mesh.indices_ptr + (triangle_idx * 12);
                m_access_manager->add_scatter_read(m_scatter_handle, idx_addr + 0, &indices[triangle_idx * 3 + 0], sizeof(int));
                m_access_manager->add_scatter_read(m_scatter_handle, idx_addr + 4, &indices[triangle_idx * 3 + 1], sizeof(int));
                m_access_manager->add_scatter_read(m_scatter_handle, idx_addr + 8, &indices[triangle_idx * 3 + 2], sizeof(int));
            }
            
            if (!m_access_manager->scatter_read(m_scatter_handle)) {
                return;
            }
        }

        // Create triangles
        for (int i = 0; i < mesh.triangle_count; i++) {
            int idx1 = indices[i * 3 + 0];
            int idx2 = indices[i * 3 + 1];
            int idx3 = indices[i * 3 + 2];

            if (idx1 < vertices.size() && idx2 < vertices.size() && idx3 < vertices.size()) {
                m_triangles.emplace_back(vertices[idx1], vertices[idx2], vertices[idx3], static_cast<uint32_t>(m_triangles.size()));
            }
        }

    } catch (const std::exception& e) {
        logger::debug("Exception processing triangle mesh: " + std::string(e.what()));
    }
}
