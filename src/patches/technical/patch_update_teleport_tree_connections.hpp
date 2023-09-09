#pragma once

#include <landstalker-lib/patches/game_patch.hpp>

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

        // 1) Build a catalog of original tree map connections that are usable to represent the new tree connections
        std::vector<MapConnection*> map_connection_pool;
        for(const auto& pair : randomizer_world.teleport_tree_pairs())
        {
            WorldTeleportTree* tree_1 = pair.first;
            WorldTeleportTree* tree_2 = pair.second;

            for(MapConnection* conn : world.map_connections(tree_1->map_id(), tree_1->paired_map_id()))
                if(!vectools::contains(map_connection_pool, conn))
                    map_connection_pool.emplace_back(conn);

            for(MapConnection* conn : world.map_connections(tree_2->map_id(), tree_2->paired_map_id()))
                if(!vectools::contains(map_connection_pool, conn))
                    map_connection_pool.emplace_back(conn);
        }

        // 2) Neutralize those connections in case they would end up unused
        for(MapConnection* connection : map_connection_pool)
        {
            connection->map_id_1(0x7FFF);
            connection->map_id_2(0x7FFF);
        }

        // 3) Set the proper map IDs to establish a warp connection between the teleport trees
        uint8_t i=0;
        for(const auto& pair : randomizer_world.teleport_tree_pairs())
        {
            WorldTeleportTree* tree_1 = pair.first;
            WorldTeleportTree* tree_2 = pair.second;
            if(tree_1 == tree_2)
                continue;

            tree_1->paired_map_id(tree_2->map_id());
            tree_2->paired_map_id(tree_1->map_id());

            // Pick map connections from the connection pool and set both map_ids to both ends of the pair
            MapConnection* conn_1 = map_connection_pool[i++];
            conn_1->map_id_1(tree_1->map_id());
            conn_1->map_id_2(tree_2->map_id());

            MapConnection* conn_2 = map_connection_pool[i++];
            conn_2->map_id_1(tree_2->map_id());
            conn_2->map_id_2(tree_1->map_id());
        }
    }
};


