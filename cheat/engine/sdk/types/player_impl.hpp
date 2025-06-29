#pragma once
#include <engine/sdk/math/vector.hpp>
#include <engine/sdk/types/entity_impl.hpp>

struct PlayerBone {
    Vector3 position;
    float padding[1];
};

struct BoundingBox {
    Vector3 mins;
    Vector3 maxs;

    void to_world(const Vector3& origin) {
        mins = mins + origin;
        maxs = maxs + origin;
    }
};

class Player : public GameEntity {
public:
    Player() : GameEntity() {}
    Player(int idx, uintptr_t list_entry) : GameEntity(idx, list_entry) {}
    Player(const GameEntity& entity) : GameEntity(entity) {}

    static constexpr int MAX_BONES = 256;

    uintptr_t pawn = 0;
    uintptr_t player_list_entry = 0;
    uintptr_t base_entity = 0;
    uintptr_t collision = 0;
    uintptr_t bone_array = 0;
    uintptr_t clipping_weapon = 0;
    uintptr_t weapon_vdata = 0;
    uintptr_t weapon_nameptr = 0;

    int team = 0;
    int health = 0;
    int armor = 0;
    bool has_helmet = false;
    bool has_defuser = false;
    bool has_c4 = false;
    bool is_visible = false;

    BoundingBox bounds = {};
    PlayerBone bones[MAX_BONES] = {};

    char playername_buffer[64] = {};
    char weaponname_buffer[64] = {};
    std::string player_name = "";
    std::string weapon_name = "";
};