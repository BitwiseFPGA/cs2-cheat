#include <engine/physics/traceline.hpp>
#include <engine/sdk/math/voxel.hpp>
#include <logger/logger.hpp>

#include <algorithm>
#include <cmath>

TracelineManager::TracelineManager()
    : m_initialized(false)
    , m_device(nullptr)
    , m_world_scene(nullptr)
    , m_world_geometry(nullptr)
    , m_smoke_scene(nullptr)
    , m_smoke_geometry(nullptr)
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
        m_device = rtcNewDevice(nullptr);
        if (!m_device) {
            logger::log_failure("Traceline Manager", "Failed to create Embree device");
            return false;
        }
        
        rtcSetDeviceErrorFunction(m_device, [](void* userPtr, enum RTCError error, const char* str) {
            logger::log_failure("Embree Error", str);
        }, nullptr);
        
        m_world_triangles.clear();
        m_smoke_voxels.clear();
        m_initialized = true;
        
        logger::log_step("Traceline Manager Init", "Embree device initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        logger::log_failure("Traceline Manager", e.what());
        cleanup_embree_resources();
        return false;
    }
}

void TracelineManager::shutdown() {
    logger::info("Shutting down TracelineManager");
    
    cleanup_embree_resources();
    m_world_triangles.clear();
    m_smoke_voxels.clear();
    m_initialized = false;
    
    logger::info("TracelineManager shutdown completed");
}

void TracelineManager::cleanup_world_resources() {
    if (m_world_geometry) {
        rtcReleaseGeometry(m_world_geometry);
        m_world_geometry = nullptr;
    }
    
    if (m_world_scene) {
        rtcReleaseScene(m_world_scene);
        m_world_scene = nullptr;
    }
}

void TracelineManager::cleanup_smoke_resources() {
    if (m_smoke_geometry) {
        rtcReleaseGeometry(m_smoke_geometry);
        m_smoke_geometry = nullptr;
    }
    
    if (m_smoke_scene) {
        rtcReleaseScene(m_smoke_scene);
        m_smoke_scene = nullptr;
    }
}

void TracelineManager::cleanup_embree_resources() {
    cleanup_world_resources();
    cleanup_smoke_resources();
    
    if (m_device) {
        rtcReleaseDevice(m_device);
        m_device = nullptr;
    }
}

bool TracelineManager::create_world_scene() {
    if (!m_device || m_world_triangles.empty()) {
        return false;
    }
    
    cleanup_world_resources();
    
    m_world_scene = rtcNewScene(m_device);
    if (!m_world_scene) {
        logger::log_failure("Traceline Manager", "Failed to create world scene");
        return false;
    }
    
    m_world_geometry = rtcNewGeometry(m_device, RTC_GEOMETRY_TYPE_TRIANGLE);
    if (!m_world_geometry) {
        logger::log_failure("Traceline Manager", "Failed to create world geometry");
        return false;
    }
    
    size_t vertex_count = m_world_triangles.size() * 3;
    float* vertices = (float*)rtcSetNewGeometryBuffer(
        m_world_geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), vertex_count
    );
    
    if (!vertices) {
        logger::log_failure("Traceline Manager", "Failed to allocate world vertex buffer");
        return false;
    }
    
    size_t triangle_count = m_world_triangles.size();
    unsigned int* indices = (unsigned int*)rtcSetNewGeometryBuffer(
        m_world_geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(unsigned int), triangle_count
    );
    
    if (!indices) {
        logger::log_failure("Traceline Manager", "Failed to allocate world index buffer");
        return false;
    }
    
    for (size_t i = 0; i < m_world_triangles.size(); ++i) {
        const Triangle& tri = m_world_triangles[i];
        
        for (int j = 0; j < 3; ++j) {
            size_t vertex_idx = i * 3 + j;
            vertices[vertex_idx * 3 + 0] = tri.vertices[j].x;
            vertices[vertex_idx * 3 + 1] = tri.vertices[j].y;
            vertices[vertex_idx * 3 + 2] = tri.vertices[j].z;
        }
        
        indices[i * 3 + 0] = static_cast<unsigned int>(i * 3 + 0);
        indices[i * 3 + 1] = static_cast<unsigned int>(i * 3 + 1);
        indices[i * 3 + 2] = static_cast<unsigned int>(i * 3 + 2);
    }
    
    rtcCommitGeometry(m_world_geometry);
    rtcAttachGeometry(m_world_scene, m_world_geometry);
    rtcCommitScene(m_world_scene);
    
    logger::debug("Created world scene with " + std::to_string(triangle_count) + " triangles");
    return true;
}

bool TracelineManager::create_smoke_scene() {
    if (!m_device || m_smoke_voxels.empty()) {
        cleanup_smoke_resources();
        return true; // Empty smoke scene is valid
    }
    
    cleanup_smoke_resources();
    
    // Generate cube triangles for each voxel
    std::vector<Triangle> smoke_triangles;
    for (const auto& voxel : m_smoke_voxels) {
        if (voxel.has_smoke && voxel.density > 0.01f) { // Only include voxels with meaningful density
            auto cube_triangles = generate_voxel_cube_triangles(voxel.world_position, 20.0f); // 20 unit cube size
            smoke_triangles.insert(smoke_triangles.end(), cube_triangles.begin(), cube_triangles.end());
        }
    }
    
    if (smoke_triangles.empty()) {
        return true; // No smoke geometry to create
    }
    
    m_smoke_scene = rtcNewScene(m_device);
    if (!m_smoke_scene) {
        logger::log_failure("Traceline Manager", "Failed to create smoke scene");
        return false;
    }
    
    m_smoke_geometry = rtcNewGeometry(m_device, RTC_GEOMETRY_TYPE_TRIANGLE);
    if (!m_smoke_geometry) {
        logger::log_failure("Traceline Manager", "Failed to create smoke geometry");
        return false;
    }
    
    size_t vertex_count = smoke_triangles.size() * 3;
    float* vertices = (float*)rtcSetNewGeometryBuffer(
        m_smoke_geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), vertex_count
    );
    
    if (!vertices) {
        logger::log_failure("Traceline Manager", "Failed to allocate smoke vertex buffer");
        return false;
    }
    
    size_t triangle_count = smoke_triangles.size();
    unsigned int* indices = (unsigned int*)rtcSetNewGeometryBuffer(
        m_smoke_geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(unsigned int), triangle_count
    );
    
    if (!indices) {
        logger::log_failure("Traceline Manager", "Failed to allocate smoke index buffer");
        return false;
    }
    
    for (size_t i = 0; i < smoke_triangles.size(); ++i) {
        const Triangle& tri = smoke_triangles[i];
        
        for (int j = 0; j < 3; ++j) {
            size_t vertex_idx = i * 3 + j;
            vertices[vertex_idx * 3 + 0] = tri.vertices[j].x;
            vertices[vertex_idx * 3 + 1] = tri.vertices[j].y;
            vertices[vertex_idx * 3 + 2] = tri.vertices[j].z;
        }
        
        indices[i * 3 + 0] = static_cast<unsigned int>(i * 3 + 0);
        indices[i * 3 + 1] = static_cast<unsigned int>(i * 3 + 1);
        indices[i * 3 + 2] = static_cast<unsigned int>(i * 3 + 2);
    }
    
    rtcCommitGeometry(m_smoke_geometry);
    rtcAttachGeometry(m_smoke_scene, m_smoke_geometry);
    rtcCommitScene(m_smoke_scene);
    
    logger::debug("Created smoke scene with " + std::to_string(triangle_count) + " triangles from " + std::to_string(m_smoke_voxels.size()) + " voxels");
    return true;
}

std::vector<Triangle> TracelineManager::generate_voxel_cube_triangles(const Vector3& center, float size) const {
    std::vector<Triangle> triangles;
    float half_size = size * 0.5f;
    
    // Define the 8 vertices of a cube
    Vector3 vertices[8] = {
        Vector3(center.x - half_size, center.y - half_size, center.z - half_size), // 0: front-bottom-left
        Vector3(center.x + half_size, center.y - half_size, center.z - half_size), // 1: front-bottom-right
        Vector3(center.x + half_size, center.y + half_size, center.z - half_size), // 2: front-top-right
        Vector3(center.x - half_size, center.y + half_size, center.z - half_size), // 3: front-top-left
        Vector3(center.x - half_size, center.y - half_size, center.z + half_size), // 4: back-bottom-left
        Vector3(center.x + half_size, center.y - half_size, center.z + half_size), // 5: back-bottom-right
        Vector3(center.x + half_size, center.y + half_size, center.z + half_size), // 6: back-top-right
        Vector3(center.x - half_size, center.y + half_size, center.z + half_size)  // 7: back-top-left
    };
    
    // Define the 12 triangles (2 per face, 6 faces)
    int face_indices[12][3] = {
        // Front face (z = -half_size)
        {0, 1, 2}, {0, 2, 3},
        // Back face (z = +half_size)
        {4, 6, 5}, {4, 7, 6},
        // Left face (x = -half_size)
        {0, 3, 7}, {0, 7, 4},
        // Right face (x = +half_size)
        {1, 5, 6}, {1, 6, 2},
        // Bottom face (y = -half_size)
        {0, 4, 5}, {0, 5, 1},
        // Top face (y = +half_size)
        {3, 2, 6}, {3, 6, 7}
    };
    
    for (int i = 0; i < 12; ++i) {
        triangles.emplace_back(
            vertices[face_indices[i][0]],
            vertices[face_indices[i][1]],
            vertices[face_indices[i][2]],
            static_cast<uint32_t>(i)
        );
    }
    
    return triangles;
}

void TracelineManager::rebuild_world_scene(std::vector<Triangle>& new_triangles) {
    if (!m_initialized) {
        return;
    }
    
    logger::debug("Rebuilding world scene for traceline");
    
    m_world_triangles = new_triangles;
    
    if (!create_world_scene()) {
        logger::log_failure("Traceline Manager", "Failed to rebuild world scene");
    }
}

void TracelineManager::rebuild_smoke_scene(const std::vector<VoxelData>& voxels) {
    if (!m_initialized) {
        return;
    }
    
    logger::debug("Rebuilding smoke scene for traceline with " + std::to_string(voxels.size()) + " voxels");
    
    m_smoke_voxels = voxels;
    
    if (!create_smoke_scene()) {
        logger::log_failure("Traceline Manager", "Failed to rebuild smoke scene");
    }
}

void TracelineManager::clear_smoke_scene() {
    if (!m_initialized) {
        return;
    }
    
    logger::debug("Clearing smoke scene for traceline");
    
    m_smoke_voxels.clear();
    cleanup_smoke_resources();
}

bool TracelineManager::is_visible_world_only(const Vector3& start, const Vector3& end) const {
    if (!m_initialized || !m_world_scene) {
        return true; // No world geometry means no obstruction
    }
    
    Vector3 direction = end - start;
    float distance = direction.length();
    
    if (distance < 0.001f) {
        return true;
    }
    
    direction = direction / distance;
    
    RTCRayHit rayhit;
    rayhit.ray.org_x = start.x;
    rayhit.ray.org_y = start.y;
    rayhit.ray.org_z = start.z;
    rayhit.ray.dir_x = direction.x;
    rayhit.ray.dir_y = direction.y;
    rayhit.ray.dir_z = direction.z;
    rayhit.ray.tnear = 0.0f;
    rayhit.ray.tfar = distance - 0.001f;
    rayhit.ray.mask = -1;
    rayhit.ray.flags = 0;
    rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
    
    rtcIntersect1(m_world_scene, &rayhit);
    
    return rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID;
}

bool TracelineManager::is_visible_smoke_only(const Vector3& start, const Vector3& end) const {
    if (!m_initialized || !m_smoke_scene) {
        return true; // No smoke geometry means no obstruction
    }
    
    Vector3 direction = end - start;
    float distance = direction.length();
    
    if (distance < 0.001f) {
        return true;
    }
    
    direction = direction / distance;
    
    RTCRayHit rayhit;
    rayhit.ray.org_x = start.x;
    rayhit.ray.org_y = start.y;
    rayhit.ray.org_z = start.z;
    rayhit.ray.dir_x = direction.x;
    rayhit.ray.dir_y = direction.y;
    rayhit.ray.dir_z = direction.z;
    rayhit.ray.tnear = 0.0f;
    rayhit.ray.tfar = distance - 0.001f;
    rayhit.ray.mask = -1;
    rayhit.ray.flags = 0;
    rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
    
    rtcIntersect1(m_smoke_scene, &rayhit);
    
    return rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID;
}

bool TracelineManager::is_visible(const Vector3& start, const Vector3& end) const {
    if (!m_initialized) {
        return true;
    }
    
    // Check world geometry first (most common obstruction)
    if (!is_visible_world_only(start, end)) {
        return false;
    }
    
    // If world doesn't block, check smoke
    if (!is_visible_smoke_only(start, end)) {
        return false;
    }
    
    // Both world and smoke are clear
    return true;
}

