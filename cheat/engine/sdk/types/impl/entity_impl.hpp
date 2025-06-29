#pragma once
#include <engine/sdk/types/base/entity_base.hpp>
#include <engine/sdk/math/vector.hpp>

class GameEntity : public BaseEntity {
public:
    GameEntity(uintptr_t address = 0);
    virtual ~GameEntity() = default;
    
    bool update() override;

protected:
    virtual bool update_transform();
    virtual bool update_health();
    virtual bool update_basic_info();
}; 