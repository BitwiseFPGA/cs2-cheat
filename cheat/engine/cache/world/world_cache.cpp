#include <engine/cache/world/world_cache.hpp>
#include <access/access.hpp>
#include <logger/logger.hpp>

WorldCache::WorldCache(AccessManager* access_manager)
    : m_access_manager(access_manager)
    , m_initialized(false)
    , m_triangles_dirty(false)
    , m_last_update(std::chrono::milliseconds(0))
{
}

WorldCache::~WorldCache() {
    if (m_initialized) {
        shutdown();
    }
    logger::info("WorldCache destructor called");
}

bool WorldCache::initialize() {
    logger::log_step("World Cache Init", "Setting up world data management");
    
    try {
        if (!m_access_manager) {
            logger::log_failure("World Cache", "Memory manager is null");
            return false;
        }
        
        clear();
        m_initialized = true;

        return true;
        
    } catch (const std::exception& e) {
        logger::log_failure("World Cache", e.what());
        return false;
    }
}

void WorldCache::shutdown() {
    logger::info("Shutting down WorldCache");
    
    clear();
    m_initialized = false;
    
    logger::info("WorldCache shutdown completed");
}

void WorldCache::update() {
    if (!m_initialized || !m_access_manager || !m_access_manager->is_attached()) {
        return;
    }
    
    try {
        logger::debug("Updating world cache");
        load_world_triangles();
        
        m_last_update = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
            
    } catch (const std::exception& e) {
        logger::error("WorldCache update failed: " + std::string(e.what()));
    }
}

void WorldCache::clear() {
    logger::debug("Clearing world cache");
    m_triangles.clear();
    m_triangles_dirty = true;
}

void Triangle::calculate_normal() {
    Vector3 edge1 = vertices[1] - vertices[0];
    Vector3 edge2 = vertices[2] - vertices[0];
    normal = edge1.cross(edge2).normalized();
}

void WorldCache::load_world_triangles() {
    //logger::debug("Loading world triangles");
}
