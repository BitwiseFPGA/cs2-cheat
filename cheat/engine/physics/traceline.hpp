#pragma once
#include <engine/sdk/math/vector.hpp>
#include <engine/sdk/math/triangle.hpp>

#include <memory>
#include <vector>

#include <embree4/rtcore.h>
#pragma comment(lib, "embree4.lib")

class VoxelData;

class TracelineManager {
public:
    TracelineManager();
    ~TracelineManager();
    
    bool initialize();
    void shutdown();
    
    // World geometry scene management
    void rebuild_world_scene(std::vector<Triangle>& new_triangles);
    
    // Smoke voxels scene management  
    void rebuild_smoke_scene(const std::vector<VoxelData>& voxels);
    void clear_smoke_scene();
    
    // Legacy method for backward compatibility
    void rebuild(std::vector<Triangle>& new_triangles) { rebuild_world_scene(new_triangles); }
    
    // Individual scene visibility checks
    bool is_visible_world_only(const Vector3& start, const Vector3& end) const;
    bool is_visible_smoke_only(const Vector3& start, const Vector3& end) const;
    
    // Global visibility check (checks both world and smoke scenes)
    bool is_visible(const Vector3& start, const Vector3& end) const;
        
private:
    bool m_initialized;
    
    // World geometry data
    std::vector<Triangle> m_world_triangles;
    RTCScene m_world_scene;
    RTCGeometry m_world_geometry;
    
    // Smoke voxels data
    std::vector<VoxelData> m_smoke_voxels;
    RTCScene m_smoke_scene;
    RTCGeometry m_smoke_geometry;
    
    // Shared Embree device
    RTCDevice m_device;
    
    // Scene creation methods
    bool create_world_scene();
    bool create_smoke_scene();
    void cleanup_world_resources();
    void cleanup_smoke_resources();
    void cleanup_embree_resources();
    
    // Helper methods for voxel cube generation
    std::vector<Triangle> generate_voxel_cube_triangles(const Vector3& center, float size) const;
}; 