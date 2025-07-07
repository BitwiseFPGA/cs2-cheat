#pragma once
#include <engine/sdk/math/vector.hpp>

class VoxelData {
public:
	VoxelData() : worldPos(0, 0, 0), density(0.0f), hasSmoke(false) {}
    VoxelData(const Vector3& pos, float dens, bool smoke) 
		: worldPos(pos), density(dens), hasSmoke(smoke) {
	}

    Vector3 worldPos;
    float density;
    bool hasSmoke;
};