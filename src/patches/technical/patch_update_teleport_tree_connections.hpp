#pragma once

#include "landstalker_lib/patches/game_patch.hpp"

#include "../../logic_model/randomizer_world.hpp"
#include "../../logic_model/world_teleport_tree.hpp"

/**
 * This patch updates the map connections between the teleport tree maps to reflect potential changes that have
 * been made to the teleport tree pairs during generation (either through the "shuffleTrees" option or through
 * explicit choice of trees in plando)
 */
class PatchUpdateTeleportTreeConnections : public GamePatch
{
public:
    void alter_world(World& world) override
    {
        const RandomizerWorld& randomizer_world = reinterpret_cast<RandomizerWorld&>(world);

        for(const auto& pair : randomizer_world.teleport_tree_pairs())
        {
            WorldTeleportTree* tree_1 = pair.first;
            WorldTeleportTree* tree_2 = pair.second;

            // If the pair is unchanged compared to vanilla, no need to do anything
            if(tree_1->paired_map_id() == tree_2->map_id())
                continue;

            // Update connections between tree maps
            for(MapConnection* conn : world.map_connections(tree_1->map_id(), tree_1->paired_map_id()))
                conn->replace_map(tree_1->paired_map_id(), tree_2->map_id());
            for(MapConnection* conn : world.map_connections(tree_2->map_id(), tree_2->paired_map_id()))
                conn->replace_map(tree_2->paired_map_id(), tree_1->map_id());

            tree_1->paired_map_id(tree_2->map_id());
            tree_2->paired_map_id(tree_1->map_id());
        }

    }
};


