#include <engine/cache/world/world_cache.hpp>
#include <access/access.hpp>
#include <logger/logger.hpp>

WorldCache::WorldCache(AccessManager* access_manager, TracelineManager* traceline_manager, Engine* engine)
    : m_access_manager(access_manager)
    , m_traceline_manager(traceline_manager)
    , m_engine(engine)
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

        m_client_dll_base = m_access_manager->get_module_base("client.dll");
        if (!m_client_dll_base) {
            logger::error("Failed to get client.dll base address");
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
        m_tracemng = m_access_manager->read<uintptr_t>(m_client_dll_base + physx::trace_mng);
        if (!m_tracemng) {
            logger::error("Failed to read trace manager");
            return;
        }

        m_physx_world_ptr = m_access_manager->read<uintptr_t>(m_tracemng);
        if (!m_physx_world_ptr) {
            logger::error("Failed to read physx world pointer");
            return;
        }

        m_physx_world = m_access_manager->read<uintptr_t>(m_physx_world_ptr + 0x30);
        if (!m_physx_world) {
            logger::error("Failed to read physx world");
            return;
        }

        m_spatial_trees_base = m_access_manager->read<uintptr_t>(m_physx_world + physx::spatial_trees_base);
        if (!m_spatial_trees_base) {
            logger::error("Failed to read spatial trees base");
            return;
        }

        if (m_prev_spatial_trees_base == m_spatial_trees_base) {
            logger::debug("Spatial trees base has not changed");
            return;
        }

        m_triangles.clear();

        std::vector<physx::SpatialTree> spatial_trees(3);
        for (int i = 0; i < 3; i++) {
            auto& tree = spatial_trees[i];
            tree.base = m_spatial_trees_base + (i * 40);
            m_access_manager->add_scatter_read(m_scatter_handle, tree.base, &tree.root_index, sizeof(tree.root_index));
            m_access_manager->add_scatter_read(m_scatter_handle, tree.base + 0x10, &tree.nodes_array, sizeof(tree.nodes_array));
            m_access_manager->add_scatter_read(m_scatter_handle, tree.base + 0x18, &tree.array_size, sizeof(tree.array_size));
        }
        m_access_manager->scatter_read(m_scatter_handle);

        size_t total_nodes = 0;
        for (const auto& tree : spatial_trees) {
            if (tree.array_size <= 0 || tree.array_size > 100000) {
                logger::warning("Invalid tree array size: " + std::to_string(tree.array_size));
                continue;
            }
            total_nodes += tree.array_size;
        }

        if (total_nodes == 0) {
            logger::warning("No valid nodes found in spatial trees");
            return;
        }

        logger::debug("Loading " + std::to_string(total_nodes) + " nodes from " + std::to_string(spatial_trees.size()) + " trees");

        std::vector<physx::TreeNode> nodes(total_nodes);
        size_t node_offset = 0;

        for (const auto& tree : spatial_trees) {
            if (tree.array_size <= 0) continue;

            for (int i = 0; i < tree.array_size; i++) {
                m_access_manager->add_scatter_read(m_scatter_handle,
                    tree.nodes_array + (i * sizeof(physx::TreeNode)),
                    &nodes[node_offset + i], sizeof(physx::TreeNode));
            }
            node_offset += tree.array_size;
        }
        m_access_manager->scatter_read(m_scatter_handle);

        std::vector<physx::NodeGeometry> geometries(total_nodes);
        size_t leaf_count = 0;

        for (size_t i = 0; i < total_nodes; i++) {
            if (nodes[i].child_or_leaf == -1 && nodes[i].geometry_ptr) {
                m_access_manager->add_scatter_read(m_scatter_handle,
                    nodes[i].geometry_ptr,
                    &geometries[i], sizeof(geometries[i]));
                leaf_count++;
            }
        }

        if (leaf_count == 0) {
            logger::warning("No leaf nodes with geometry found");
            return;
        }

        logger::debug("Found " + std::to_string(leaf_count) + " leaf nodes with geometry");
        m_access_manager->scatter_read(m_scatter_handle);

        std::vector<physx::ConvexHull> hulls(total_nodes);
        std::vector<physx::TriangleMesh> meshes(total_nodes);

        for (size_t i = 0; i < total_nodes; i++) {
            if (nodes[i].child_or_leaf == -1 && nodes[i].geometry_ptr) {
                if (geometries[i].shape_ptr) {
                    m_access_manager->add_scatter_read(m_scatter_handle,
                        geometries[i].shape_ptr,
                        &hulls[i], sizeof(hulls[i]));
                }
                if (geometries[i].mesh_data) {
                    m_access_manager->add_scatter_read(m_scatter_handle,
                        geometries[i].mesh_data,
                        &meshes[i], sizeof(meshes[i]));
                }
            }
        }
        m_access_manager->scatter_read(m_scatter_handle);

        std::vector<std::vector<Vector3>> all_hull_vertices(total_nodes);
        std::vector<std::vector<Vector3>> all_mesh_vertices(total_nodes);
        std::vector<std::vector<int>> all_mesh_indices(total_nodes);

        size_t hull_count = 0, mesh_count = 0;

        for (size_t i = 0; i < total_nodes; i++) {
            if (nodes[i].child_or_leaf != -1) continue;

            if (hulls[i].vertices_ptr && hulls[i].vertex_count > 2 && hulls[i].vertex_count < 10000) {
                all_hull_vertices[i].resize(hulls[i].vertex_count);
                for (int j = 0; j < hulls[i].vertex_count; j++) {
                    uintptr_t vertex_addr = hulls[i].vertices_ptr + (j * sizeof(Vector3));
                    m_access_manager->add_scatter_read(m_scatter_handle,
                        vertex_addr,
                        &all_hull_vertices[i][j], sizeof(Vector3));
                }
                hull_count++;
            }

            if (meshes[i].vertices_ptr && meshes[i].vertex_count > 0 && meshes[i].vertex_count < 100000 &&
                meshes[i].indices_ptr && meshes[i].triangle_count > 0 && meshes[i].triangle_count < 100000) {

                all_mesh_vertices[i].resize(meshes[i].vertex_count);
                for (int j = 0; j < meshes[i].vertex_count; j++) {
                    uintptr_t vertex_addr = meshes[i].vertices_ptr + (j * sizeof(Vector3));
                    m_access_manager->add_scatter_read(m_scatter_handle,
                        vertex_addr,
                        &all_mesh_vertices[i][j], sizeof(Vector3));
                }

                all_mesh_indices[i].resize(meshes[i].triangle_count * 3);
                for (int j = 0; j < meshes[i].triangle_count; j++) {
                    uintptr_t triangle_base = meshes[i].indices_ptr + (j * 3 * sizeof(int));
                    m_access_manager->add_scatter_read(m_scatter_handle,
                        triangle_base,
                        &all_mesh_indices[i][j * 3], 3 * sizeof(int));
                }
                mesh_count++;
            }
        }

        logger::debug("Processing " + std::to_string(hull_count) + " hulls and " + std::to_string(mesh_count) + " meshes");
        m_access_manager->scatter_read(m_scatter_handle);

        size_t triangle_count = 0;

        for (size_t i = 0; i < total_nodes; i++) {
            if (nodes[i].child_or_leaf != -1) continue;

            if (!all_hull_vertices[i].empty()) {
                const auto& vertices = all_hull_vertices[i];
                if (vertices.size() >= 3) {
                    for (size_t j = 1; j < vertices.size() - 1; j++) {
                        m_triangles.emplace_back(
                            vertices[0],
                            vertices[j],
                            vertices[j + 1],
                            static_cast<uint32_t>(triangle_count++)
                        );
                    }
                }
            }

            if (!all_mesh_vertices[i].empty() && !all_mesh_indices[i].empty()) {
                const auto& vertices = all_mesh_vertices[i];
                const auto& indices = all_mesh_indices[i];

                for (int j = 0; j < meshes[i].triangle_count; j++) {
                    int idx1 = indices[j * 3 + 0];
                    int idx2 = indices[j * 3 + 1];
                    int idx3 = indices[j * 3 + 2];

                    // Validate indices
                    if (idx1 >= 0 && idx1 < static_cast<int>(vertices.size()) &&
                        idx2 >= 0 && idx2 < static_cast<int>(vertices.size()) &&
                        idx3 >= 0 && idx3 < static_cast<int>(vertices.size())) {

                        m_triangles.emplace_back(
                            vertices[idx1],
                            vertices[idx2],
                            vertices[idx3],
                            static_cast<uint32_t>(triangle_count++)
                        );
                    }
                }
            }
        }

        if (m_triangles.empty()) {
            logger::warning("No triangles generated from world cache");
            return;
        }

        m_prev_spatial_trees_base = m_spatial_trees_base;
        logger::info("Successfully loaded " + std::to_string(m_triangles.size()) +
            " triangles from " + std::to_string(leaf_count) + " leaf nodes");

        m_traceline_manager->rebuild_world_scene(m_triangles);

    }
    catch (const std::exception& e) {
        logger::error("Exception in load_world_triangles: " + std::string(e.what()));
    }
}