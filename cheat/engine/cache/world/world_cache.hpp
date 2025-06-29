#pragma once
#include <logger/logger.hpp>
#include <engine/sdk/math/vector.hpp>
#include <engine/sdk/math/matrix.hpp>
#include <engine/sdk/math/triangle.hpp>
#include <vector>
#include <memory>
#include <chrono>

class AccessManager;

class WorldCache {
public:
    WorldCache(AccessManager* access_manager);
    ~WorldCache();
    
    bool initialize();
    void shutdown();
    
    void update();
    void clear();
    
    const std::vector<Triangle>& get_triangles() const { return m_triangles; }
    
    bool triangles_updated() const { return m_triangles_dirty; }
    void mark_triangles_clean() { m_triangles_dirty = false; }
    
private:
    AccessManager* m_access_manager;
    bool m_initialized;
    bool m_triangles_dirty;
    
    std::chrono::milliseconds m_last_update;
    
    std::vector<Triangle> m_triangles;
    
    void load_world_triangles();
};

