#pragma once
#include <engine/sdk/math/vector.hpp>
#include <cstdint>

struct Triangle {
    Vector3 vertices[3];
    Vector3 normal;
    uint32_t id;
    uint32_t material_id;
    
    Triangle() : id(0), material_id(0) {}
    Triangle(const Vector3& v0, const Vector3& v1, const Vector3& v2, uint32_t triangle_id = 0, uint32_t mat_id = 0)
        : id(triangle_id), material_id(mat_id) {
        vertices[0] = v0;
        vertices[1] = v1;
        vertices[2] = v2;
        calculate_normal();
    }
    
    void calculate_normal();
};
