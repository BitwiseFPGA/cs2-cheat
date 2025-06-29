#pragma once
#include <engine/sdk/types/impl/entity_impl.hpp>

class BasePlayer : public GameEntity {
public:
    BasePlayer() = default;
    virtual ~BasePlayer() = default;
    
    virtual bool update() = 0;
}; 