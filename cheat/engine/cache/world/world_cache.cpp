#include <engine/cache/world/world_cache.hpp>
#include <access/access.hpp>
#include <logger/logger.hpp>

WorldCache::WorldCache(AccessManager* access_manager)
    : m_access_manager(access_manager)
    , m_initialized(false)
    , m_triangles_dirty(false)
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
    m_triangles_dirty = true;
}

/*
uintptr_t tracemng = Mem::Read<uintptr_t>(Cache::client_dll + physx::trace_mng);
if (!tracemng) return;

uintptr_t physx_world_ptr = Mem::Read<uintptr_t>(tracemng);
if (!physx_world_ptr) return;

uintptr_t physx_world = Mem::Read<uintptr_t>(physx_world_ptr + 0x30);
if (!physx_world) return;
if ((physx_world == prev_physx_world) && prev_physx_world != 0) return;

prev_physx_world = physx_world;

// Clear previous map data
ClearMap();

uintptr_t spatial_trees_base = Mem::Read<uintptr_t>(physx_world + physx::spatial_trees_base);
if (!spatial_trees_base) return;

for (int tree_index = 0; tree_index < 3; tree_index++) {
	uintptr_t spatial_tree = spatial_trees_base + (tree_index * 40);

	int root_index = Mem::Read<int>(spatial_tree);

	uintptr_t nodes_array = Mem::Read<uintptr_t>(spatial_tree + 0x10);
	int array_size = Mem::Read<int>(spatial_tree + 0x18);

	if (root_index < 0 || !nodes_array || array_size <= 0) continue;

	std::stack<int> node_stack;
	std::unordered_set<int> visited;
	node_stack.push(root_index);

	while (!node_stack.empty()) {
		int current_node = node_stack.top();
		node_stack.pop();

		if (current_node < 0 || current_node >= array_size || visited.count(current_node)) {
			continue;
		}
		visited.insert(current_node);

		uintptr_t node_ptr = nodes_array + (current_node * sizeof(physx::TreeNode));
		physx::TreeNode node = Mem::Read<physx::TreeNode>(node_ptr);

		if (node.child_or_leaf == -1) {
			if (node.geometry_ptr) {
				physx::NodeGeometry geometry = Mem::Read<physx::NodeGeometry>(node.geometry_ptr);

				if (geometry.shape_ptr) {
					physx::ConvexHull hull = Mem::Read<physx::ConvexHull>(geometry.shape_ptr);
					if (hull.vertices_ptr && hull.vertex_count > 2 && hull.vertex_count < 10000) {
						std::vector<Vector3> vertices;
						vertices.reserve(hull.vertex_count);

						for (int i = 0; i < hull.vertex_count; i++) {
							Vector3 vertex = Mem::Read<Vector3>(hull.vertices_ptr + (i * sizeof(Vector3)));
							vertices.push_back(vertex);
						}

						for (int i = 1; i < vertices.size() - 1; i++) {
							triangles.emplace_back(vertices[0], vertices[i], vertices[i + 1]);
						}
					}
				}

				if (geometry.mesh_data) {
					physx::TriangleMesh mesh = Mem::Read<physx::TriangleMesh>(geometry.mesh_data);
					if (mesh.vertices_ptr && mesh.vertex_count > 0 &&
						mesh.indices_ptr && mesh.triangle_count > 0 &&
						mesh.vertex_count < 100000 && mesh.triangle_count < 100000) {

						std::vector<Vector3> vertices;
						vertices.reserve(mesh.vertex_count);

						for (int i = 0; i < mesh.vertex_count; i++) {
							Vector3 vertex = Mem::Read<Vector3>(mesh.vertices_ptr + (i * sizeof(Vector3)));
							vertices.push_back(vertex);
						}

						for (int i = 0; i < mesh.triangle_count; i++) {
							int idx1 = Mem::Read<int>(mesh.indices_ptr + (i * 12) + 0);
							int idx2 = Mem::Read<int>(mesh.indices_ptr + (i * 12) + 4);
							int idx3 = Mem::Read<int>(mesh.indices_ptr + (i * 12) + 8);

							if (idx1 < vertices.size() && idx2 < vertices.size() && idx3 < vertices.size()) {
								triangles.emplace_back(vertices[idx1], vertices[idx2], vertices[idx3]);
							}
						}
					}
				}
			}
		}
		else {
			node_stack.push(node.child_or_leaf);
			if (node.sibling_index >= 0 && node.sibling_index < array_size) {
				node_stack.push(node.sibling_index);
			}
		}
	}
}

BuildGrid();
*/
void WorldCache::load_world_triangles() {
    uintptr_t tracemng = m_access_manager->read<uintptr_t>(physx::trace_mng);
}
