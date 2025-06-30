#pragma once
#include <engine/sdk/math/vector.hpp>
#include <engine/sdk/math/triangle.hpp>
#include <memory>
#include <vector>

#include <embree4/rtcore.h>
#pragma comment(lib, "embree4.lib")

class TracelineManager {
public:
    TracelineManager();
    ~TracelineManager();
    
    bool initialize();
    void shutdown();
    
    void rebuild(std::vector<Triangle>& new_triangles);
    
    bool is_visible(const Vector3& start, const Vector3& end) const;
        
private:
    bool m_initialized;
    
    std::vector<Triangle> m_triangles;
    
}; 