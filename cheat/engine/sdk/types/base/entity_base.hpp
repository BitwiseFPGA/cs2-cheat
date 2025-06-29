#pragma once
#include <cstdint>
#include <access/access.hpp>

class BaseEntity {
public:
    BaseEntity() = default;
    virtual ~BaseEntity() = default;
    
    virtual bool update() = 0;
}; 