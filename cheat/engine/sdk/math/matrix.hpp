#pragma once
#include <engine/sdk/math/vector.hpp>
#include <array>

struct Matrix4x4 {
    float m[4][4];
    
    Matrix4x4() {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                m[i][j] = (i == j) ? 1.0f : 0.0f;
            }
        }
    }
    
    Matrix4x4(const std::array<std::array<float, 4>, 4>& matrix) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                m[i][j] = matrix[i][j];
            }
        }
    }
    
    Matrix4x4 operator*(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i][j] = 0.0f;
                for (int k = 0; k < 4; k++) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }
    
    Vector3 transform_point(const Vector3& point) const {
        float w = m[3][0] * point.x + m[3][1] * point.y + m[3][2] * point.z + m[3][3];
        if (w == 0.0f) w = 1.0f;
        
        return Vector3(
            (m[0][0] * point.x + m[0][1] * point.y + m[0][2] * point.z + m[0][3]) / w,
            (m[1][0] * point.x + m[1][1] * point.y + m[1][2] * point.z + m[1][3]) / w,
            (m[2][0] * point.x + m[2][1] * point.y + m[2][2] * point.z + m[2][3]) / w
        );
    }
    
    Vector3 transform_direction(const Vector3& direction) const {
        return Vector3(
            m[0][0] * direction.x + m[0][1] * direction.y + m[0][2] * direction.z,
            m[1][0] * direction.x + m[1][1] * direction.y + m[1][2] * direction.z,
            m[2][0] * direction.x + m[2][1] * direction.y + m[2][2] * direction.z
        );
    }
    
    static Matrix4x4 identity() {
        return Matrix4x4();
    }
    
    static Matrix4x4 translation(const Vector3& translation) {
        Matrix4x4 result;
        result.m[0][3] = translation.x;
        result.m[1][3] = translation.y;
        result.m[2][3] = translation.z;
        return result;
    }
};

