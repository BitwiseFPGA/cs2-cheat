#include <engine/cache/entity/entity_cache.hpp>
#include <access/access.hpp>
#include <logger/logger.hpp>
#include <engine/sdk/offsets/static/client_dll.hpp>
#include <engine/sdk/offsets/static/offsets.hpp>
#include <algorithm>
#include <cstring>

inline uint32_t fnv1a_32(const char* str) {
    uint32_t hash = 0x811C9DC5;
    while (*str) {
        hash ^= (uint8_t)*str++;
        hash *= 0x1000193;
    }
    return hash;
}

EntityCache::EntityCache(AccessManager* access_manager)
    : m_access_manager(access_manager)
    , m_initialized(false)
    , m_last_update(std::chrono::milliseconds(0))
{
}

EntityCache::~EntityCache() {
    if (m_initialized) {
        shutdown();
    }
    logger::info("EntityCache destructor called");
}

bool EntityCache::initialize() {
    logger::log_step("Entity Cache Init", "Setting up entity management");
    
    try {
        if (!m_access_manager) {
            logger::log_failure("Entity Cache", "Memory manager is null");
            return false;
        }

        m_scatter_handle = m_access_manager->create_scatter_handle();
        if (!m_scatter_handle) {
            logger::log_failure("Entity Cache", "Failed to create scatter handle");
            return false;
        }
        
        m_client_dll_base = m_access_manager->get_module_base("client.dll");
        if (!m_client_dll_base) {
            logger::log_failure("Entity Cache", "Failed to get client.dll base");
            return false;
        }

        logger::log_value("client.dll", m_client_dll_base);

        clear();
        m_initialized = true;

        return true;
        
    } catch (const std::exception& e) {
        logger::log_failure("Entity Cache", e.what());
        return false;
    }
}

void EntityCache::shutdown() {
    logger::info("Shutting down EntityCache");
    
    if (m_scatter_handle && m_access_manager) {
        m_access_manager->close_scatter_handle(m_scatter_handle);
        m_scatter_handle = nullptr;
    }
    
    clear();
    m_initialized = false;
    
    logger::info("EntityCache shutdown completed");
}

void EntityCache::update() {
    if (!m_initialized || !m_access_manager || !m_access_manager->is_attached()) {
        return;
    }
    
    try {
        fetch_globals();
        fetch_entities();
        fetch_other_entity_data(m_entities);
		update_frame();

        m_last_update = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
            
    } catch (const std::exception& e) {
        logger::error("EntityCache update failed: " + std::string(e.what()));
    }
}

void EntityCache::fetch_globals() {
    if (!m_scatter_handle) {
        return;
    }

    m_access_manager->add_scatter_read(
        m_scatter_handle,
        m_client_dll_base + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn,
        &m_local_player_ptr,
        sizeof(m_local_player_ptr)
    );

    m_access_manager->add_scatter_read(
        m_scatter_handle,
        m_client_dll_base + cs2_dumper::offsets::client_dll::dwEntityList,
        &m_entity_list_ptr,
        sizeof(m_entity_list_ptr)
    );

    m_access_manager->add_scatter_read(
        m_scatter_handle,
        m_client_dll_base + cs2_dumper::offsets::client_dll::dwGlobalVars,
        &m_global_vars_ptr,
        sizeof(m_global_vars_ptr)
    );

    m_access_manager->scatter_read(m_scatter_handle);
}

void EntityCache::fetch_entities() {
    if (!m_scatter_handle || m_entity_list_ptr == 0) {
        return;
    }

    m_list_entries.resize(MAX_ENTITIES);

    m_access_manager->add_scatter_read(
        m_scatter_handle,
        m_entity_list_ptr + 0x10,
        m_list_entries.data(),
        m_list_entries.size() * sizeof(uintptr_t)
    );

    m_access_manager->scatter_read(m_scatter_handle);

    auto make_key = [](uintptr_t list_entry, int idx) -> uint64_t {
        return (static_cast<uint64_t>(list_entry) << 32) | static_cast<uint64_t>(idx);
    };

    m_new_entities_buffer.clear();
    m_new_entities_buffer.reserve(MAX_ENTITIES);
    m_entities_to_update_buffer.clear();
    m_entities_to_update_buffer.reserve(MAX_ENTITIES);

    for (int i = 0; i < MAX_ENTITIES; i++) {
        int list_index = (i & 0x7FFF) >> 9;
        if (list_index >= static_cast<int>(m_list_entries.size())) {
            break;
        }
        
        auto& entity = m_new_entities_buffer.emplace_back(i, m_list_entries[list_index]);
        
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            entity.list_entry + 120LL * (i & 0x1FF), 
            &entity.ptr, 
            sizeof(entity.ptr)
        );
    }

    m_access_manager->scatter_read(m_scatter_handle);

    for (auto& entity : m_new_entities_buffer) {
        if (entity.ptr == 0) {
            continue;
        }
        
        uint64_t key = make_key(entity.list_entry, entity.idx);
        
        auto it = m_entity_cache_buffer.find(key);
        if (it != m_entity_cache_buffer.end() && it->second.ptr == entity.ptr) {
            entity = it->second;
        } else {
            m_entities_to_update_buffer.push_back(&entity);
        }
    }

    if (!m_entities_to_update_buffer.empty()) {
        fetch_entity_data(m_entities_to_update_buffer);
    }

    m_new_players_buffer.clear();
    m_new_players_buffer.reserve(MAX_PLAYERS + 1);
    
    std::vector<GameEntity*> player_controllers;
    for (const auto& entity : m_new_entities_buffer) {
        if (entity.classname_hash == fnv1a_32("CCSPlayerController")) {
            player_controllers.push_back(const_cast<GameEntity*>(&entity));
        }
    }
    
    if (player_controllers.empty()) {
        m_players = std::move(m_new_players_buffer);
        return;
    }
    
    std::vector<uint32_t> current_pawn_handles(player_controllers.size());
    for (size_t i = 0; i < player_controllers.size(); ++i) {
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            player_controllers[i]->ptr + cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn, 
            &current_pawn_handles[i], 
            sizeof(current_pawn_handles[i])
        );
    }
    m_access_manager->scatter_read(m_scatter_handle);
    
    m_players_to_update_buffer.clear();
    m_players_to_update_buffer.reserve(MAX_PLAYERS + 1);
    
    for (size_t i = 0; i < player_controllers.size(); ++i) {
        const auto& entity = *player_controllers[i];
        uint64_t key = make_key(entity.list_entry, entity.idx);
        
        auto it = m_player_cache_buffer.find(key);
        if (it != m_player_cache_buffer.end() && it->second.pawn == current_pawn_handles[i]) {
            m_new_players_buffer.push_back(it->second);
        } else {
            auto& player = m_new_players_buffer.emplace_back(entity);
            player.pawn = current_pawn_handles[i];
            m_players_to_update_buffer.push_back(&player);
        }
    }

    if (!m_players_to_update_buffer.empty()) {
        fetch_player_data(m_players_to_update_buffer);
    }

    m_entities = std::move(m_new_entities_buffer);
    m_players = std::move(m_new_players_buffer);

    m_entity_cache_buffer.clear();
    m_player_cache_buffer.clear();
    m_entity_cache_buffer.reserve(m_entities.size());
    m_player_cache_buffer.reserve(m_players.size());

    for (const auto& entity : m_entities) {
        if (entity.ptr != 0) {
            m_entity_cache_buffer[make_key(entity.list_entry, entity.idx)] = entity;
        }
    }
    
    for (const auto& player : m_players) {
        if (player.ptr != 0) {
            m_player_cache_buffer[make_key(player.list_entry, player.idx)] = player;
        }
    }
}

void EntityCache::fetch_entity_data(std::vector<GameEntity*>& entities_to_update) {
    if (!m_scatter_handle || entities_to_update.empty()) {
        return;
    }

    for (auto* entity : entities_to_update) {
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            entity->ptr + cs2_dumper::schemas::client_dll::CEntityInstance::m_pEntity, 
            &entity->instance, 
            sizeof(entity->instance)
        );
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            entity->ptr + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode, 
            &entity->gamescene_node, 
            sizeof(entity->gamescene_node)
        );
    }
    m_access_manager->scatter_read(m_scatter_handle);

    for (auto* entity : entities_to_update) {
        if (entity->instance == 0) {
            continue;
        }
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            entity->instance + 0x8, 
            &entity->class_info, 
            sizeof(entity->class_info)
        );
    }
    m_access_manager->scatter_read(m_scatter_handle);

    for (auto* entity : entities_to_update) {
        if (entity->class_info == 0) {
            continue;
        }
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            entity->class_info + 0x28, 
            &entity->schema_class, 
            sizeof(entity->schema_class)
        );
    }
    m_access_manager->scatter_read(m_scatter_handle);

    for (auto* entity : entities_to_update) {
        if (entity->schema_class == 0) {
            continue;
        }
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            entity->schema_class + 0x8, 
            &entity->classname_address, 
            sizeof(entity->classname_address)
        );
    }
    m_access_manager->scatter_read(m_scatter_handle);

    for (auto* entity : entities_to_update) {
        if (entity->classname_address == 0) {
            continue;
        }
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            entity->classname_address, 
            entity->classname_buffer, 
            sizeof(entity->classname_buffer)
        );
    }
    m_access_manager->scatter_read(m_scatter_handle);

    for (auto* entity : entities_to_update) {
        entity->classname = std::string(entity->classname_buffer);
        entity->classname_hash = fnv1a_32(entity->classname.c_str());
    }
}

void EntityCache::fetch_player_data(std::vector<Player*>& players_to_update) {
    if (!m_scatter_handle || players_to_update.empty() || m_entity_list_ptr == 0) {
        return;
    }

    for (auto* player : players_to_update) {
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            m_entity_list_ptr + 8 * ((player->pawn & 0x7FFF) >> 9) + 0x10, 
            &player->player_list_entry, 
            sizeof(player->player_list_entry)
        );
    }
    m_access_manager->scatter_read(m_scatter_handle);

    for (auto* player : players_to_update) {
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            player->player_list_entry + 120 * (player->pawn & 0x1FF), 
            &player->base_entity, 
            sizeof(player->base_entity)
        );
    }
    m_access_manager->scatter_read(m_scatter_handle);

    for (auto* player : players_to_update) {
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            player->base_entity + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pCollision, 
            &player->collision, 
            sizeof(player->collision)
        );
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            player->base_entity + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode, 
            &player->gamescene_node, 
            sizeof(player->gamescene_node)
        );
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            player->base_entity + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum, 
            &player->team, 
            sizeof(player->team)
        );
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            player->base_entity + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth, 
            &player->health, 
            sizeof(player->health)
        );
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            player->base_entity + cs2_dumper::schemas::client_dll::C_CSPlayerPawnBase::m_pClippingWeapon, 
            &player->clipping_weapon, 
            sizeof(player->clipping_weapon)
        );
    }
    m_access_manager->scatter_read(m_scatter_handle);

    if (players_to_update.empty()) {
        return;
    }

    for (auto* player : players_to_update) {
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            player->gamescene_node + 0x170 + 0x80, 
            &player->bone_array, 
            sizeof(player->bone_array)
        );
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            player->ptr + cs2_dumper::schemas::client_dll::CBasePlayerController::m_iszPlayerName, 
            player->playername_buffer, 
            sizeof(player->playername_buffer)
        );
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            player->clipping_weapon + 0x380, 
            &player->weapon_vdata, 
            sizeof(player->weapon_vdata)
        );
    }
    m_access_manager->scatter_read(m_scatter_handle);

    for (auto* player : players_to_update) {
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            player->weapon_vdata + cs2_dumper::schemas::client_dll::CCSWeaponBaseVData::m_szName, 
            &player->weapon_nameptr, 
            sizeof(player->weapon_nameptr)
        );
    }
    m_access_manager->scatter_read(m_scatter_handle);

    for (auto* player : players_to_update) {
        m_access_manager->add_scatter_read(
            m_scatter_handle, 
            player->weapon_nameptr, 
            player->weaponname_buffer, 
            sizeof(player->weaponname_buffer)
        );
    }
    m_access_manager->scatter_read(m_scatter_handle);

    for (auto* player : players_to_update) {
        player->player_name = std::string(player->playername_buffer);
        player->weapon_name = std::string(player->weaponname_buffer);
    }
}

void EntityCache::fetch_other_entity_data(const std::vector<GameEntity>& entities) {
    if (!m_scatter_handle || entities.empty()) {
        return;
    }
    m_smokes.clear();

    std::vector<SmokeGrenade> smoke_entities_to_update;
    for (const auto& entity : entities) {
        if (entity.classname_hash == fnv1a_32("C_SmokeGrenadeProjectile")) {
            smoke_entities_to_update.emplace_back(entity);
        }
    }

    if (smoke_entities_to_update.empty()) {
        return;
    }

    for (auto& smoke : smoke_entities_to_update) {
        smoke.object_smoke_ptr = smoke.ptr + 0x1268;
        m_access_manager->add_scatter_read(
            m_scatter_handle,
            smoke.object_smoke_ptr + 0xE8,
            &smoke.smoke_center,
            sizeof(smoke.smoke_center)
        );
        m_access_manager->add_scatter_read(
            m_scatter_handle,
            smoke.object_smoke_ptr + 0x70,
            &smoke.voxel_grid_base,
            sizeof(smoke.voxel_grid_base)
        );
        m_access_manager->add_scatter_read(
            m_scatter_handle,
            smoke.object_smoke_ptr + 0x100,
            &smoke.buffer_index,
            sizeof(smoke.buffer_index)
        );
    }
    m_access_manager->scatter_read(m_scatter_handle);

    std::vector<std::vector<uint64_t>> occupancy_data(smoke_entities_to_update.size());
    for (size_t smoke_idx = 0; smoke_idx < smoke_entities_to_update.size(); ++smoke_idx) {
        auto& smoke = smoke_entities_to_update[smoke_idx];
        occupancy_data[smoke_idx].resize(SmokeGrenade::NUM_CHUNKS);
        
        for (int chunk_idx = 0; chunk_idx < SmokeGrenade::NUM_CHUNKS; chunk_idx++) {
            m_access_manager->add_scatter_read(
                m_scatter_handle,
                smoke.GetOccupancyAddress(chunk_idx),
                &occupancy_data[smoke_idx][chunk_idx],
                sizeof(uint64_t)
            );
        }
    }
    m_access_manager->scatter_read(m_scatter_handle);

    std::vector<std::vector<uint32_t>> occupied_voxels(smoke_entities_to_update.size());
    std::vector<std::vector<float>> density_data(smoke_entities_to_update.size());
    
    size_t total_voxels = 0;
    for (size_t smoke_idx = 0; smoke_idx < smoke_entities_to_update.size(); ++smoke_idx) {
        auto& smoke = smoke_entities_to_update[smoke_idx];
        occupied_voxels[smoke_idx] = smoke.GetOccupiedVoxelIndices(occupancy_data[smoke_idx]);
        density_data[smoke_idx].resize(occupied_voxels[smoke_idx].size());
        total_voxels += occupied_voxels[smoke_idx].size();
    }

    if (total_voxels > 0) {
        for (size_t smoke_idx = 0; smoke_idx < smoke_entities_to_update.size(); ++smoke_idx) {
            auto& smoke = smoke_entities_to_update[smoke_idx];
            for (size_t voxel_idx = 0; voxel_idx < occupied_voxels[smoke_idx].size(); ++voxel_idx) {
                uint32_t voxel_index = occupied_voxels[smoke_idx][voxel_idx];
                m_access_manager->add_scatter_read(
                    m_scatter_handle,
                    smoke.GetDensityAddress(voxel_index),
                    &density_data[smoke_idx][voxel_idx],
                    sizeof(float)
                );
            }
        }
        m_access_manager->scatter_read(m_scatter_handle);

        for (size_t smoke_idx = 0; smoke_idx < smoke_entities_to_update.size(); ++smoke_idx) {
            auto& smoke = smoke_entities_to_update[smoke_idx];
            smoke.ProcessVoxelData(occupied_voxels[smoke_idx], density_data[smoke_idx]);
        }
    }

    m_smokes = std::move(smoke_entities_to_update);
}

void EntityCache::update_frame() {
    if (!m_scatter_handle) {
        return;
    }

    m_c4 = nullptr;
    m_local_player = nullptr;

    for (auto& entity : m_entities) {
        if (entity.gamescene_node != 0) {
            m_access_manager->add_scatter_read(
                m_scatter_handle, 
                entity.gamescene_node + cs2_dumper::schemas::client_dll::CGameSceneNode::m_vecOrigin, 
                &entity.origin, 
                sizeof(entity.origin)
            );
        }

        if (entity.classname_hash == fnv1a_32("C_C4")) {
            m_c4 = &entity;
            m_access_manager->add_scatter_read(
                m_scatter_handle,
                entity.ptr + cs2_dumper::schemas::client_dll::C_BaseEntity::m_hOwnerEntity,
                &entity.owner_ptr,
                sizeof(entity.owner_ptr)
            );
        }
    }

    for (auto& player : m_players) {
        m_access_manager->add_scatter_read(
           m_scatter_handle, 
           player.gamescene_node + cs2_dumper::schemas::client_dll::CGameSceneNode::m_vecOrigin, 
           &player.origin, 
           sizeof(player.origin)
       );
       m_access_manager->add_scatter_read(
           m_scatter_handle, 
           player.base_entity + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth, 
           &player.health, 
           sizeof(player.health)
       );
       m_access_manager->add_scatter_read(
           m_scatter_handle, 
           player.base_entity + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_ArmorValue, 
           &player.armor, 
           sizeof(player.armor)
       );
       m_access_manager->add_scatter_read(
           m_scatter_handle, 
           player.collision + cs2_dumper::schemas::client_dll::CCollisionProperty::m_vecMins, 
           &player.bounds, 
           sizeof(player.bounds)
       );
       m_access_manager->add_scatter_read(
           m_scatter_handle, 
           player.ptr + cs2_dumper::schemas::client_dll::CCSPlayerController::m_bPawnHasHelmet, 
           &player.has_helmet, 
           sizeof(player.has_helmet)
       );
       m_access_manager->add_scatter_read(
           m_scatter_handle, 
           player.ptr + cs2_dumper::schemas::client_dll::CCSPlayerController::m_bPawnHasDefuser, 
           &player.has_defuser, 
           sizeof(player.has_defuser)
       );
       m_access_manager->add_scatter_read(
           m_scatter_handle, 
           player.base_entity + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum, 
           &player.team, 
           sizeof(player.team)
       );
       m_access_manager->add_scatter_read(
           m_scatter_handle, 
           player.bone_array, 
           player.bones, 
           std::min<size_t>(sizeof(player.bones), static_cast<size_t>(Player::MAX_BONES * sizeof(PlayerBone)))
       );

       if (player.base_entity == m_local_player_ptr) {
           m_local_player = &player;
       }
    }
    m_access_manager->scatter_read(m_scatter_handle);

    for (auto& player : m_players) {
        player.bounds.to_world(player.origin);
    }

    if (m_c4) {
        for (auto& player : m_players) {
            if ((m_c4->owner_ptr & 0x7FFF) == (player.pawn & 0x7FFF)) {
                player.has_c4 = true;
            } else {
                player.has_c4 = false;
            }
        }
    } else {
        for (auto& player : m_players) {
            player.has_c4 = false;
        }
    }

}

void EntityCache::clear() {
    logger::debug("Clearing entity cache");
    
    m_entities.clear();
    m_players.clear();
    m_smokes.clear();
    m_list_entries.clear();
    
    m_local_player = nullptr;
    m_c4 = nullptr;
    
    m_entity_cache_buffer.clear();
    m_player_cache_buffer.clear();
    m_new_entities_buffer.clear();
    m_new_players_buffer.clear();
    m_entities_to_update_buffer.clear();
    m_players_to_update_buffer.clear();
}
