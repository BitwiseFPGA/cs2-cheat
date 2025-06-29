#include <engine/cache/entity/entity_cache.hpp>
#include <access/access.hpp>
#include <logger/logger.hpp>
#include <algorithm>

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

void EntityCache::update() {
    if (!m_initialized || !m_access_manager || !m_access_manager->is_attached()) {
        return;
    }
    
    try {
        logger::debug("Updating entity cache");


        m_last_update = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
            
    } catch (const std::exception& e) {
        logger::error("EntityCache update failed: " + std::string(e.what()));
    }
}

void EntityCache::clear() {
    logger::debug("Clearing entity cache");
    
    m_entities.clear();
    m_players.clear();
    m_local_player.reset();
}
