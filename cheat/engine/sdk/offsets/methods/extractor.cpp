#include <engine/sdk/offsets/methods/extractor.hpp>
#include <logger/logger.hpp>

OffsetExtractor::OffsetExtractor(AccessManager* access_manager) : m_access_manager(access_manager) { }

bool OffsetExtractor::extract_offsets() {
    uintptr_t client_base = m_access_manager->get_module_base("client.dll");

    if (!client_base) {
        return false;
    }
    
    uint32_t smoke_object_offset = m_access_manager->find_pattern_and_extract<uint32_t>(client_base, "48 81 C1 ? ? ? ? E8 ? ? ? ? 83 F8 02 75 07 C6 83 ? ? ? ? ? 83 F8 01 0F 94 C0 48 83 C4 20 5B C3", 3);
    logger::log_value("smoke_object_offset: ", smoke_object_offset, true);
    
    uint32_t smoke_center_offset = m_access_manager->find_pattern_and_extract<uint32_t>(client_base, "F2 0F 10 96 ? ? ? ? 48 8B 5C 24 ? F3 0F 10 00 F3 0F 5C 05 ? ? ? ? F3 0F 10 48 ? F3 0F 5C 0D ? ? ? ? 48 8B 6C 24 ? F3 0F 59 C3", 4);
    logger::log_value("smoke_center_offset: ", smoke_center_offset, true);
    
    uint16_t smoke_voxel_grid_base_offset = 0x70;
    logger::log_value("smoke_voxel_grid_base_offset: ", smoke_voxel_grid_base_offset, true);
    
    uint32_t smoke_buffer_index_offset = m_access_manager->find_pattern_and_extract<uint32_t>(client_base, "48 63 AB ? ? ? ? BF ? ? ? ? 4C 8B 5B 70 4C 8B C5 44 8B C8 44 8B D0 83 E0 3F 0F B6 C8 49 C1 E9 06 48 8B C5", 3);
    logger::log_value("smoke_buffer_index_offset: ", smoke_buffer_index_offset, true);
    
    uint32_t smoke_occupancy_address_offset = m_access_manager->find_pattern_and_extract<uint32_t>(client_base, "4A 8B 84 C1 ? ? ? ? 4C 0F AB D0 4A 89 84 C1 ? ? ? ? 48 83 C6 0C 49 83 EE 01", 4);
    logger::log_value("smoke_occupancy_address_offset: ", smoke_occupancy_address_offset, true);
    
    uint32_t smoke_density_address_offset = m_access_manager->find_pattern_and_extract<uint32_t>(client_base, "F3 43 0F 58 84 C3 ? ? ? ? F3 43 0F 11 84 C3 ? ? ? ? F3 41 0F 11 84 CB ? ? ? ? 4C 63 83 ? ? ? ? 48 8B 4B 70 49 C1 E0 09 4D 03 C1", 6);
    logger::log_value("smoke_density_address_offset: ", smoke_density_address_offset, true);
    
    uint32_t weapon_vdata_offset = m_access_manager->find_pattern_and_extract<uint32_t>(client_base, "48 8B 87 ? ? ? ? 48 85 C0 48 0F 45 D8 48 81 C3 ? ? ? ? EB 60", 3);
    logger::log_value("weapon_vdata_offset: ", weapon_vdata_offset, true);
    
    uint32_t bone_array_offset = m_access_manager->find_pattern_and_extract<uint32_t>(client_base, "4C 03 B3 ? ? ? ? 0F 84 ? ? ? ? 90 80 BB ? ? ? ? ? 75 0C 48 8B CB E8 ? ? ? ? 84 C0 74 08 ", 3);
    logger::log_value("bone_array_offset: ", bone_array_offset, true);

    m_offsets["smoke_object_offset"] = smoke_object_offset;
	m_offsets["smoke_center_offset"] = smoke_center_offset;
	m_offsets["smoke_voxel_grid_base_offset"] = smoke_voxel_grid_base_offset;
	m_offsets["smoke_buffer_index_offset"] = smoke_buffer_index_offset;
	m_offsets["smoke_occupancy_address_offset"] = smoke_occupancy_address_offset;
	m_offsets["smoke_density_address_offset"] = smoke_density_address_offset;
	m_offsets["weapon_vdata_offset"] = weapon_vdata_offset;
	m_offsets["bone_array_offset"] = bone_array_offset;

    return true;
}

uint32_t OffsetExtractor::get_offset(const std::string& name) {
    auto it = m_offsets.find(name);
    if (it != m_offsets.end()) {
        return it->second;
    }
    return 0;
}

