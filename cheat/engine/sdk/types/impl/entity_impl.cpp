#include <engine/sdk/types/impl/entity_impl.hpp>
#include <logger/logger.hpp>

GameEntity::GameEntity(uintptr_t address)
{
}

bool GameEntity::update() {
    try {
        if (!update_transform()) return false;
        if (!update_health()) return false;
        if (!update_basic_info()) return false;
        
        return true;
        
    } catch (const std::exception& e) {
        logger::error("GameEntity update failed: " + std::string(e.what()));
        return false;
    }
}

bool GameEntity::update_transform() {
    return true;
}

bool GameEntity::update_health() {
    return true;
}

bool GameEntity::update_basic_info() {
    return true;
} 