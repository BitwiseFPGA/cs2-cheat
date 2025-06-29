#pragma once
#include <engine/sdk/types/base/player_base.hpp>
#include <engine/sdk/types/impl/entity_impl.hpp>

class GamePlayer : public BasePlayer {
public:
    GamePlayer(uintptr_t address = 0);
    virtual ~GamePlayer() = default;
    
    bool update() override;

protected:
    virtual bool update_player_state();
    virtual bool update_combat_info();
    virtual bool update_bone_positions();
    virtual bool update_visibility();
}; 