#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <landstalker-lib/model/map.hpp>
#include <landstalker-lib/constants/map_codes.hpp>

/**
 * This patch optimizes a few laggy Tibor map by removing a few ghosts which are almost unnoticeable.
 */
class PatchOptimizeTiborMaps : public GamePatch
{
public:
    void alter_world(World& world) override
    {
        // Remove a few ghosts from laggy Tibor maps
        world.map(MAP_TIBOR_808)->remove_entity(5);

        world.map(MAP_TIBOR_811)->remove_entity(4);

        world.map(MAP_TIBOR_812)->remove_entity(2);

        world.map(MAP_TIBOR_815)->remove_entity(3);
    }

};