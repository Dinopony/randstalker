#pragma once

#include <landstalker_lib/patches/game_patch.hpp>
#include <landstalker_lib/constants/map_codes.hpp>
#include <landstalker_lib/model/map.hpp>
#include <landstalker_lib/model/entity.hpp>

class PatchFixArmletSkip : public GamePatch
{
public:
    void alter_world(World& world) override
    {
        // Make Mir Tower magic barrier non-colliding
        world.map(MAP_MIR_TOWER_EXTERIOR)->entity(9)->can_pass_through(true);
    }
};
