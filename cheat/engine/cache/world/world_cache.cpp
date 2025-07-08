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
        uintptr_t tracemng = 0;
        uintptr_t physx_world_ptr = 0;
        uintptr_t physx_world = 0;
        uintptr_t spatial_trees_base = 0;

        m_access_manager->add_scatter_read(m_scatter_handle, m_client_dll_base + physx::trace_mng, &tracemng, sizeof(tracemng));
        if (!m_access_manager->scatter_read(m_scatter_handle)) {
            logger::error("Failed to read trace manager");
            return;
        }

        if (!tracemng) {
            logger::error("Invalid trace manager address");
            return;
        }

        m_access_manager->add_scatter_read(m_scatter_handle, tracemng, &physx_world_ptr, sizeof(physx_world_ptr));
        if (!m_access_manager->scatter_read(m_scatter_handle)) {
            logger::error("Failed to read physx world pointer");
            return;
        }

        if (!physx_world_ptr) {
            logger::error("Invalid physx world pointer");
            return;
        }

        m_access_manager->add_scatter_read(m_scatter_handle, physx_world_ptr + 0x30, &physx_world, sizeof(physx_world));
        if (!m_access_manager->scatter_read(m_scatter_handle)) {
            logger::error("Failed to read physx world");
            return;
        }

        if (!physx_world) {
            logger::error("Invalid physx world address");
            return;
        }

        if (m_prev_physx_world == physx_world) {
            return;
        }

        m_access_manager->add_scatter_read(m_scatter_handle, physx_world + physx::spatial_trees_base, &spatial_trees_base, sizeof(spatial_trees_base));
        if (!m_access_manager->scatter_read(m_scatter_handle)) {
            logger::error("Failed to read spatial trees base");
            return;
        }

        if (!spatial_trees_base) {
            logger::error("Invalid spatial trees base address");
            return;
        }

        m_triangles.clear();

        std::vector<physx::SpatialTree> spatial_trees(3);

        for (int i = 0; i < 3; i++) {
            auto& tree = spatial_trees[i];
            tree.base = spatial_trees_base + (i * 40);
            m_access_manager->add_scatter_read(m_scatter_handle, tree.base, &tree.root_index, sizeof(tree.root_index));
            m_access_manager->add_scatter_read(m_scatter_handle, tree.base + 0x10, &tree.nodes_array, sizeof(tree.nodes_array));
            m_access_manager->add_scatter_read(m_scatter_handle, tree.base + 0x18, &tree.array_size, sizeof(tree.array_size));
        }
        m_access_manager->scatter_read(m_scatter_handle);

        size_t total_nodes = 0;
        for (auto& tree : spatial_trees) {
            total_nodes += tree.array_size;
        }

        std::vector<physx::TreeNode> nodes(total_nodes);

        for (auto& tree : spatial_trees) {
            for (int i = 0; i < tree.array_size; i++) {
                m_access_manager->add_scatter_read(m_scatter_handle, tree.nodes_array + (i * sizeof(physx::TreeNode)), &nodes[i], sizeof(physx::TreeNode));
            }
        }
        m_access_manager->scatter_read(m_scatter_handle);

        std::vector<physx::NodeGeometry> geometries(total_nodes);

        for (int i = 0; i < total_nodes; i++) {
            if (nodes[i].child_or_leaf == -1 && nodes[i].geometry_ptr) {
                m_access_manager->add_scatter_read(m_scatter_handle, nodes[i].geometry_ptr, &geometries[i], sizeof(geometries[i]));
            }
        }
        m_access_manager->scatter_read(m_scatter_handle);

        std::vector<physx::ConvexHull> hulls(total_nodes);
        std::vector<physx::TriangleMesh> meshes(total_nodes);

        for (int i = 0; i < total_nodes; i++) {
            if (geometries[i].shape_ptr) {
                m_access_manager->add_scatter_read(m_scatter_handle, geometries[i].shape_ptr, &hulls[i], sizeof(hulls[i]));
            }
            if (geometries[i].mesh_data) {
                m_access_manager->add_scatter_read(m_scatter_handle, geometries[i].mesh_data, &meshes[i], sizeof(meshes[i]));
            }
        }
        m_access_manager->scatter_read(m_scatter_handle);

        std::vector<std::vector<Vector3>> all_hull_vertices(total_nodes);
        std::vector<std::vector<Vector3>> all_mesh_vertices(total_nodes);
        std::vector<std::vector<int>> all_mesh_indices(total_nodes);

        for (int i = 0; i < total_nodes; i++) {
            if (hulls[i].vertices_ptr && hulls[i].vertex_count > 2 && hulls[i].vertex_count < 10000) {
                all_hull_vertices[i].resize(hulls[i].vertex_count);
                for (int j = 0; j < hulls[i].vertex_count; j++) {
                    uintptr_t vertex_addr = hulls[i].vertices_ptr + (j * sizeof(Vector3));
                    m_access_manager->add_scatter_read(m_scatter_handle, vertex_addr, &all_hull_vertices[i][j], sizeof(Vector3));
                }
            }

            if (meshes[i].vertices_ptr && meshes[i].vertex_count > 0 && meshes[i].vertex_count < 100000 &&
                meshes[i].indices_ptr && meshes[i].triangle_count > 0 && meshes[i].triangle_count < 100000) {

                all_mesh_vertices[i].resize(meshes[i].vertex_count);
                for (int j = 0; j < meshes[i].vertex_count; j++) {
                    uintptr_t vertex_addr = meshes[i].vertices_ptr + (j * sizeof(Vector3));
                    m_access_manager->add_scatter_read(m_scatter_handle, vertex_addr, &all_mesh_vertices[i][j], sizeof(Vector3));
                }

                all_mesh_indices[i].resize(meshes[i].triangle_count * 3);
                for (int j = 0; j < meshes[i].triangle_count; j++) {
                    uintptr_t triangle_base = meshes[i].indices_ptr + (j * 3 * sizeof(int));
                    m_access_manager->add_scatter_read(m_scatter_handle, triangle_base, &all_mesh_indices[i][j * 3], 3 * sizeof(int));
                }
            }
        }
        m_access_manager->scatter_read(m_scatter_handle);

        for (int i = 0; i < total_nodes; i++) {
            if (!all_hull_vertices[i].empty()) {
                const auto& vertices = all_hull_vertices[i];
                for (int j = 1; j < vertices.size() - 1; j++) {
                    m_triangles.emplace_back(vertices[0], vertices[j], vertices[j + 1], static_cast<uint32_t>(m_triangles.size()));
                }
            }

            if (!all_mesh_vertices[i].empty() && !all_mesh_indices[i].empty()) {
                const auto& vertices = all_mesh_vertices[i];
                const auto& indices = all_mesh_indices[i];

                for (int j = 0; j < meshes[i].triangle_count; j++) {
                    int idx1 = indices[j * 3 + 0];
                    int idx2 = indices[j * 3 + 1];
                    int idx3 = indices[j * 3 + 2];

                    if (idx1 >= 0 && idx1 < static_cast<int>(vertices.size()) &&
                        idx2 >= 0 && idx2 < static_cast<int>(vertices.size()) &&
                        idx3 >= 0 && idx3 < static_cast<int>(vertices.size())) {

                        m_triangles.emplace_back(
                            vertices[idx1],
                            vertices[idx2],
                            vertices[idx3],
                            static_cast<uint32_t>(m_triangles.size())
                        );
                    }
                }
            }
        }

        if (m_triangles.empty()) {
            logger::debug("No triangles found in world cache");
            return;
        }
        else {
            m_prev_physx_world = physx_world;
            logger::debug("Loaded " + std::to_string(m_triangles.size()) + " triangles from world cache");
            m_traceline_manager->rebuild(m_triangles);
        }

    }
    catch (const std::exception& e) {
        logger::error("Exception in load_world_triangles: " + std::string(e.what()));
    }
}
