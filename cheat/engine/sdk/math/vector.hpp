#pragma once
#include <cmath>
#include <imgui.h>

struct Vector2 : public ImVec2 {    
    Vector2() : ImVec2() {}
    Vector2(float x, float y) : ImVec2(x, y) {}
    
    Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
    Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
    Vector2 operator*(float scalar) const { return Vector2(x * scalar, y * scalar); }
    Vector2 operator/(float scalar) const { return Vector2(x / scalar, y / scalar); }
    
    Vector2& operator+=(const Vector2& other) { x += other.x; y += other.y; return *this; }
    Vector2& operator-=(const Vector2& other) { x -= other.x; y -= other.y; return *this; }
    Vector2& operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
    Vector2& operator/=(float scalar) { x /= scalar; y /= scalar; return *this; }
    
    float length() const { return sqrt(x * x + y * y); }
    float length_squared() const { return x * x + y * y; }
    Vector2 normalized() const { float len = length(); return len > 0 ? *this / len : Vector2(); }
    float dot(const Vector2& other) const { return x * other.x + y * other.y; }
};

struct Vector3 {
    float x, y, z;
    
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
    Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
    Vector3 operator*(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
    Vector3 operator/(float scalar) const { return Vector3(x / scalar, y / scalar, z / scalar); }
    
    Vector3& operator+=(const Vector3& other) { x += other.x; y += other.y; z += other.z; return *this; }
    Vector3& operator-=(const Vector3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
    Vector3& operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
    Vector3& operator/=(float scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }

	bool operator==(const Vector3& other) const { return x == other.x && y == other.y && z == other.z; }
	bool operator!=(const Vector3& other) const { return !(*this == other); }
    
    float length() const { return sqrt(x * x + y * y + z * z); }
    float length_squared() const { return x * x + y * y + z * z; }
    Vector3 normalized() const { float len = length(); return len > 0 ? *this / len : Vector3(); }
    float dot(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }
    Vector3 cross(const Vector3& other) const { 
        return Vector3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x); 
    }
    
    float distance_to(const Vector3& other) const { return (*this - other).length(); }
    float distance_squared_to(const Vector3& other) const { return (*this - other).length_squared(); }
};

struct Vector4 {
    float x, y, z, w;
    
    Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    
    Vector4 operator+(const Vector4& other) const { return Vector4(x + other.x, y + other.y, z + other.z, w + other.w); }
    Vector4 operator-(const Vector4& other) const { return Vector4(x - other.x, y - other.y, z - other.z, w - other.w); }
    Vector4 operator*(float scalar) const { return Vector4(x * scalar, y * scalar, z * scalar, w * scalar); }
    Vector4 operator/(float scalar) const { return Vector4(x / scalar, y / scalar, z / scalar, w / scalar); }
    
    Vector4& operator+=(const Vector4& other) { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
    Vector4& operator-=(const Vector4& other) { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
    Vector4& operator*=(float scalar) { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
    Vector4& operator/=(float scalar) { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }
    
    float length() const { return sqrt(x * x + y * y + z * z + w * w); }
    float length_squared() const { return x * x + y * y + z * z + w * w; }
    Vector4 normalized() const { float len = length(); return len > 0 ? *this / len : Vector4(); }
	float dot(const Vector4& other) const { return x * other.x + y * other.y + z * other.z + w * other.w; }
    Vector4 cross(const Vector4& other) const { 
        return Vector4(
            y * other.z - z * other.y, 
            z * other.x - x * other.z, 
            x * other.y - y * other.x, 
            0.0f
        ); 
    }
    
    float distance_to(const Vector4& other) const { return (*this - other).length(); }
	float distance_squared_to(const Vector4& other) const { return (*this - other).length_squared(); }
};