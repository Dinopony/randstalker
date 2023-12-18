#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include "../../logic_model/world_teleport_tree.hpp"

/**
 * In the original game, you need to save Tibor to make teleport trees usable.
 * This patch removes this requirement.
 */
class PatchRemoveTiborRequirement : public GamePatch
{
public:
    void alter_world(World& world) override
    {
        // Clear all useless "invisible cubes" present to open the tree on the fly when a flag would be triggered
        // during map execution (which cannot happen anymore anyway, so this is only causing harm)
        RandomizerWorld& randomizer_world = reinterpret_cast<RandomizerWorld&>(world);

        for(const auto& pair : randomizer_world.teleport_tree_pairs())
        {
            WorldTeleportTree* tree_1 = pair.first;
            Map* map_1 = world.map(tree_1->map_id());
            map_1->visited_flag(tree_1->flag());
            map_1->clear_entities();

            WorldTeleportTree* tree_2 = pair.second;
            Map* map_2 = world.map(tree_2->map_id());
            map_2->visited_flag(tree_2->flag());
            map_2->clear_entities();
        }
    }

    void alter_rom(md::ROM& rom) override
    {
        // Remove the check of the "completed Tibor sidequest" flag to make trees usable
        rom.set_code(0x4E4A, md::Code().nop(5));
    }
};
