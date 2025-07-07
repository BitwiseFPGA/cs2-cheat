#include "smoke.hpp"

Vector3 SmokeGrenade::DecodeMortonIndex(uint32_t index)
{
    auto extractCoordinate = [](uint32_t value, int shift) -> uint32_t {
        value = (value >> shift) & 0x9249249;
        value = (value | (value >> 2)) & 0x30C30C3;
        value = (value | (value >> 4)) & 0x300F00F;
        value = (value | (value >> 8)) & 0xFF;
        return value;
    };

    return Vector3{
        (float)extractCoordinate(index, 0),  // x
        (float)extractCoordinate(index, 1),  // y  
        (float)extractCoordinate(index, 2)   // z
    };
}

Vector3 SmokeGrenade::VoxelIndexToWorldPos(Vector3 origin, uint32_t index)
{
    Vector3 coords = DecodeMortonIndex(index);

    const float voxelScale = 20.f;
    const int gridSize = 32;

    const float gridCenterOffset = (gridSize * voxelScale) * 0.5f;

    Vector3 worldPos;
    worldPos.x = (coords.x * voxelScale) + origin.x - gridCenterOffset;
    worldPos.y = (coords.y * voxelScale) + origin.y - gridCenterOffset;
    worldPos.z = (coords.z * voxelScale) + origin.z - gridCenterOffset;

    return worldPos;
}

uintptr_t SmokeGrenade::GetOccupancyAddress(int chunkIndex) const
{
    return voxel_grid_base + 8 * (chunkIndex + (buffer_index << 9)) + 0x1008;
}

uintptr_t SmokeGrenade::GetDensityAddress(uint32_t voxelIndex) const
{
    return voxel_grid_base + 16 * (voxelIndex + (buffer_index << 15)) + 0x3008;
}

std::vector<uint32_t> SmokeGrenade::GetOccupiedVoxelIndices(const std::vector<uint64_t>& occupancy_data) const
{
    std::vector<uint32_t> occupied_indices;
    
    for (int chunkIndex = 0; chunkIndex < NUM_CHUNKS; chunkIndex++) {
        uint64_t occupancyBits = occupancy_data[chunkIndex];
        if (occupancyBits == 0) continue;

        for (int bitIndex = 0; bitIndex < CHUNK_SIZE; bitIndex++) {
            if ((occupancyBits >> bitIndex) & 1) {
                uint32_t voxelIndex = chunkIndex * CHUNK_SIZE + bitIndex;
                occupied_indices.push_back(voxelIndex);
            }
        }
    }
    
    return occupied_indices;
}

void SmokeGrenade::ProcessVoxelData(const std::vector<uint32_t>& voxel_indices, const std::vector<float>& densities)
{
    voxels.clear();
    voxels.reserve(voxel_indices.size());
    
    for (size_t i = 0; i < voxel_indices.size() && i < densities.size(); ++i) {
        if (densities[i] <= 0.0f) continue;
        
        VoxelData voxel;
        voxel.worldPos = VoxelIndexToWorldPos(smoke_center, voxel_indices[i]);
        voxel.density = densities[i];
        voxel.hasSmoke = true;
        voxels.push_back(voxel);
    }
}
