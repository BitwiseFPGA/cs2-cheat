#pragma once
#include <engine/sdk/math/vector.hpp>

class VoxelData {
public:
	VoxelData() : world_position(0, 0, 0), density(0.0f), has_smoke(false) {}
    VoxelData(const Vector3& pos, float dens, bool smoke) 
		: world_position(pos), density(dens), has_smoke(smoke) {
	}

    Vector3 world_position;
    float density;
    bool has_smoke;
};