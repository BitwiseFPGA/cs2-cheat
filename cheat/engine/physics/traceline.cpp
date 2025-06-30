#include <engine/physics/traceline.hpp>
#include <logger/logger.hpp>
#include <algorithm>
#include <cmath>

TracelineManager::TracelineManager()
    : m_initialized(false), m_device(nullptr), m_scene(nullptr), m_geometry(nullptr)
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
        // Initialize Embree device
        m_device = rtcNewDevice(nullptr);
        if (!m_device) {
            logger::log_failure("Traceline Manager", "Failed to create Embree device");
            return false;
        }
        
        // Set error handler
        rtcSetDeviceErrorFunction(m_device, [](void* userPtr, enum RTCError error, const char* str) {
            logger::log_failure("Embree Error", str);
        }, nullptr);
        
        m_triangles.clear();
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
    m_triangles.clear();
    m_initialized = false;
    
    logger::info("TracelineManager shutdown completed");
}

void TracelineManager::cleanup_embree_resources() {
    if (m_geometry) {
        rtcReleaseGeometry(m_geometry);
        m_geometry = nullptr;
    }
    
    if (m_scene) {
        rtcReleaseScene(m_scene);
        m_scene = nullptr;
    }
    
    if (m_device) {
        rtcReleaseDevice(m_device);
        m_device = nullptr;
    }
}

bool TracelineManager::create_embree_scene() {
    if (!m_device || m_triangles.empty()) {
        return false;
    }
    
    // Clean up existing scene
    if (m_geometry) {
        rtcReleaseGeometry(m_geometry);
        m_geometry = nullptr;
    }
    
    if (m_scene) {
        rtcReleaseScene(m_scene);
        m_scene = nullptr;
    }
    
    // Create new scene
    m_scene = rtcNewScene(m_device);
    if (!m_scene) {
        logger::log_failure("Traceline Manager", "Failed to create Embree scene");
        return false;
    }
    
    // Create triangle mesh geometry
    m_geometry = rtcNewGeometry(m_device, RTC_GEOMETRY_TYPE_TRIANGLE);
    if (!m_geometry) {
        logger::log_failure("Traceline Manager", "Failed to create Embree geometry");
        return false;
    }
    
    // Set vertex buffer
    size_t vertex_count = m_triangles.size() * 3;
    float* vertices = (float*)rtcSetNewGeometryBuffer(
        m_geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, 3 * sizeof(float), vertex_count
    );
    
    if (!vertices) {
        logger::log_failure("Traceline Manager", "Failed to allocate vertex buffer");
        return false;
    }
    
    // Set index buffer
    size_t triangle_count = m_triangles.size();
    unsigned int* indices = (unsigned int*)rtcSetNewGeometryBuffer(
        m_geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, 3 * sizeof(unsigned int), triangle_count
    );
    
    if (!indices) {
        logger::log_failure("Traceline Manager", "Failed to allocate index buffer");
        return false;
    }
    
    // Fill vertex and index buffers
    for (size_t i = 0; i < m_triangles.size(); ++i) {
        const Triangle& tri = m_triangles[i];
        
        // Copy vertices
        for (int j = 0; j < 3; ++j) {
            size_t vertex_idx = i * 3 + j;
            vertices[vertex_idx * 3 + 0] = tri.vertices[j].x;
            vertices[vertex_idx * 3 + 1] = tri.vertices[j].y;
            vertices[vertex_idx * 3 + 2] = tri.vertices[j].z;
        }
        
        // Set indices
        indices[i * 3 + 0] = static_cast<unsigned int>(i * 3 + 0);
        indices[i * 3 + 1] = static_cast<unsigned int>(i * 3 + 1);
        indices[i * 3 + 2] = static_cast<unsigned int>(i * 3 + 2);
    }
    
    // Commit geometry and attach to scene
    rtcCommitGeometry(m_geometry);
    rtcAttachGeometry(m_scene, m_geometry);
    
    // Commit scene to enable ray tracing
    rtcCommitScene(m_scene);
    
    logger::debug("Created Embree scene with " + std::to_string(triangle_count) + " triangles");
    return true;
}

void TracelineManager::rebuild(std::vector<Triangle>& new_triangles) {
    if (!m_initialized) {
        return;
    }
    
    logger::debug("Rebuilding spatial optimization for traceline");
    
    m_triangles = new_triangles;
    
    // Rebuild Embree scene
    if (!create_embree_scene()) {
        logger::log_failure("Traceline Manager", "Failed to rebuild Embree scene");
    }
}

bool TracelineManager::is_visible(const Vector3& start, const Vector3& end) const {
    if (!m_initialized || !m_scene) {
        return false;
    }
    
    // Calculate ray direction and distance
    Vector3 direction = end - start;
    float distance = direction.length();
    
    if (distance < 0.001f) {
        return true; // Points are essentially the same
    }
    
    direction = direction / distance; // Normalize
    
    // Create Embree ray (following tutorial pattern)
    RTCRayHit rayhit;
    rayhit.ray.org_x = start.x;
    rayhit.ray.org_y = start.y;
    rayhit.ray.org_z = start.z;
    rayhit.ray.dir_x = direction.x;
    rayhit.ray.dir_y = direction.y;
    rayhit.ray.dir_z = direction.z;
    rayhit.ray.tnear = 0.0f;
    rayhit.ray.tfar = distance - 0.001f; // Slightly less than full distance to avoid self-intersection
    rayhit.ray.mask = -1;
    rayhit.ray.flags = 0;
    rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
    rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
    
    // Perform ray-scene intersection
    rtcIntersect1(m_scene, &rayhit);
    
    // If no intersection found, the path is clear
    return rayhit.hit.geomID == RTC_INVALID_GEOMETRY_ID;
}
