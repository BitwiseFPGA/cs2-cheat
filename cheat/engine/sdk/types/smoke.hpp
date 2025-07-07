#pragma once
#include <vector>
#include <engine/sdk/math/vector.hpp>
#include <engine/sdk/math/voxel.hpp>
#include <engine/sdk/types/entity.hpp>

class SmokeGrenade : public GameEntity {
public:
    SmokeGrenade() : GameEntity() {}
    SmokeGrenade(int idx, uintptr_t list_entry) : GameEntity(idx, list_entry) {}
    SmokeGrenade(const GameEntity& entity) : GameEntity(entity) {}

    const static uint32_t TOTAL_VOXELS = 0x8000;
    const static int CHUNK_SIZE = 64;
    const static int NUM_CHUNKS = TOTAL_VOXELS / CHUNK_SIZE;

    uintptr_t object_smoke_ptr = 0;
    uintptr_t voxel_grid_base = 0;
    int buffer_index = 0;

    Vector3 smoke_center = {};

    std::vector<uint32_t> voxel_indices = {};
    std::vector<VoxelData> voxels = {};

    // Address calculation methods
    uintptr_t GetOccupancyAddress(int chunkIndex) const;
    uintptr_t GetDensityAddress(uint32_t voxelIndex) const;
    
    // Voxel processing methods
    std::vector<uint32_t> GetOccupiedVoxelIndices(const std::vector<uint64_t>& occupancy_data) const;
    void ProcessVoxelData(const std::vector<uint32_t>& voxel_indices, const std::vector<float>& densities);
    
    // Coordinate transformation
    Vector3 VoxelIndexToWorldPos(Vector3 origin, uint32_t index);

private:
    Vector3 DecodeMortonIndex(uint32_t index);
};
