#include <engine/cache/entity/entity_cache.hpp>
#include <access/access.hpp>
#include <logger/logger.hpp>
#include <engine/sdk/offsets/static/client_dll.hpp>
#include <engine/sdk/offsets/static/offsets.hpp>

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
    
    clear();
    m_initialized = false;
    
    logger::info("EntityCache shutdown completed");
}

/*
void Cache::FetchEntities(HANDLE scatter_handle)
{
    Mem::AddScatter(
        scatter_handle,
        entity_list + 0x10,
        list_entries_buffer.data(),
        list_entries_buffer.size() * sizeof(uintptr_t)
    );

    Mem::ExecuteScatter(scatter_handle);

    for (int i = 0; i < MAX_ENTITIES; i++) {
        auto& entity = new_entities_buffer.emplace_back();
        entity.idx = i;
        entity.list_entry = list_entries_buffer[(i & 0x7FFF) >> 9];

        Mem::AddScatter(scatter_handle, entity.list_entry + 120LL * (i & 0x1FF), &entity.ptr, sizeof(entity.ptr));
    }

    Mem::ExecuteScatter(scatter_handle);

    if (!entities_to_update_buffer.empty()) {
        FetchEntityData(scatter_handle, entities_to_update_buffer);
    }


    frame_buffer.entities = std::move(new_entities_buffer);
}

void Cache::FetchEntityData(HANDLE scatter_handle, std::vector<Entity*>& entities_to_update)
{
    if (!scatter_handle || entities_to_update.empty())
        return;

    for (auto* entity : entities_to_update) {
        Mem::AddScatter(scatter_handle, entity->ptr + CEntityInstance::m_pEntity, &entity->instance, sizeof(entity->instance));
        Mem::AddScatter(scatter_handle, entity->ptr + C_BaseEntity::m_pGameSceneNode, &entity->gamescene_node, sizeof(entity->gamescene_node));
    }
    Mem::ExecuteScatter(scatter_handle);

    for (auto* entity : entities_to_update) {
        Mem::AddScatter(scatter_handle, entity->instance + 0x8, &entity->class_info, sizeof(entity->class_info));
    }
    Mem::ExecuteScatter(scatter_handle);

    for (auto* entity : entities_to_update) {
        Mem::AddScatter(scatter_handle, entity->class_info + 0x28, &entity->schema_class, sizeof(entity->schema_class));
    }
    Mem::ExecuteScatter(scatter_handle);

    for (auto* entity : entities_to_update) {
        Mem::AddScatter(scatter_handle, entity->schema_class + 0x8, &entity->classname_address, sizeof(entity->classname_address));
    }
    Mem::ExecuteScatter(scatter_handle);

    for (auto* entity : entities_to_update) {
        Mem::AddScatter(scatter_handle, entity->classname_address, entity->classname_buffer, sizeof(entity->classname_buffer));
    }
    Mem::ExecuteScatter(scatter_handle);

    for (auto* entity : entities_to_update) {
        entity->classname = std::string(entity->classname_buffer);
        entity->classname_hash = fnv1a_32(entity->classname.c_str());
    }
}
*/

void EntityCache::update() {
    if (!m_initialized || !m_access_manager || !m_access_manager->is_attached()) {
        return;
    }
    
    try {
        logger::debug("Updating entity cache");

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

        logger::log_value("m_local_player_ptr", m_local_player_ptr);
        logger::log_value("m_entity_list_ptr", m_entity_list_ptr);
        logger::log_value("m_global_vars_ptr", m_global_vars_ptr);

        m_list_entries.resize(MAX_ENTITIES);

        m_access_manager->add_scatter_read(
            m_scatter_handle,
            m_entity_list_ptr + 0x10,
            m_list_entries.data(),
            m_list_entries.size() * sizeof(uintptr_t)
        );

        m_access_manager->scatter_read(m_scatter_handle);
        
        std::vector<std::shared_ptr<GameEntity>> entities_to_update;

        for (int i = 0; i < MAX_ENTITIES; i++) {
            auto new_ent = std::make_shared<GameEntity>(i, m_list_entries[(i & 0x7FFF) >> 9]);
            entities_to_update.push_back(new_ent);
            m_access_manager->add_scatter_read(
                m_scatter_handle,
                new_ent->list_entry + 120LL * (i & 0x1FF),
                &new_ent->ptr,
                sizeof(new_ent->ptr)
            );
        }

        m_access_manager->scatter_read(m_scatter_handle);



        m_last_update = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
            
    } catch (const std::exception& e) {
        logger::error("EntityCache update failed: " + std::string(e.what()));
    }
}

void EntityCache::clear() {
    logger::debug("Clearing entity cache");
}
