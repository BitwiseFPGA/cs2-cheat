#pragma once
#include <engine/sdk/math/vector.hpp>

#include <iostream>

class GameEntity {
public:
    GameEntity() : idx(0), list_entry(0) {
        this->ptr = 0;
        this->instance = 0;
        this->class_info = 0;
        this->schema_class = 0;
        this->classname_address = 0;
    }
    
    GameEntity(int idx, uintptr_t list_entry) : idx(idx), list_entry(list_entry) {
        this->ptr = 0;
        this->instance = 0;
        this->class_info = 0;
        this->schema_class = 0;
        this->classname_address = 0;
    }
    ~GameEntity() = default;
    
public:
    int idx;
    uintptr_t list_entry;
    uintptr_t ptr = 0;
    uintptr_t instance = 0;
    uintptr_t class_info = 0;
    uintptr_t schema_class = 0;
    uintptr_t classname_address = 0;
    uintptr_t gamescene_node = 0;
    uintptr_t owner_ptr = 0;

    char classname_buffer[64] = {};
    std::string classname = "";
    uint32_t classname_hash = 0;

    Vector3 origin = { 0, 0, 0 };
}; 
