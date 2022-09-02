#pragma once

#include "landstalker_lib/patches/game_patch.hpp"
#include "landstalker_lib/model/world.hpp"
#include "landstalker_lib/model/map.hpp"
#include "landstalker_lib/model/entity.hpp"
#include "landstalker_lib/constants/map_codes.hpp"

/**
 * This patch optimizes maps, clearing maps which are unused in the randomizer while making some other run smoother.
 */
class PatchOptimizeMaps : public GamePatch
{
public:
    void alter_world(World& world) override
    {
        optimize_laggy_tibor_rooms(world);
        clear_unreachable_maps(world);
        remove_useless_entities(world);
        optimize_palettes_usage(world);
        optimize_map_connections(world);

        transform_variant_into_standard(world.map(MAP_MERCATOR_CASTLE_THRONE_ROOM_ARTHUR_VARIANT));

        remove_useless_map_variants(world);
    }

private:
    static void optimize_laggy_tibor_rooms(World& world)
    {
        // Remove ghosts from laggy Tibor rooms
        world.map(MAP_TIBOR_808)->remove_entity(5);
        world.map(MAP_TIBOR_811)->remove_entity(4);
        world.map(MAP_TIBOR_812)->remove_entity(2);
        world.map(MAP_TIBOR_813)->remove_entity(1);
        world.map(MAP_TIBOR_814)->remove_entity(2);
        world.map(MAP_TIBOR_815)->remove_entity(3);
    }

    static void clear_unreachable_maps(World& world)
    {
        // Clear unreachable maps
        const std::vector<uint16_t> UNREACHABLE_MAPS = {
                MAP_INTRO_139, MAP_INTRO_140, MAP_INTRO_141, MAP_INTRO_142, MAP_INTRO_143,
                MAP_MERCATOR_CASTLE_KAYLA_ROOM, MAP_MERCATOR_CASTLE_KAYLA_BATHROOM_ENTRANCE, MAP_MERCATOR_CASTLE_KAYLA_BATHROOM
        };

        for(uint16_t map_id : UNREACHABLE_MAPS)
            world.map(map_id)->clear();
    }

    static void remove_useless_entities(World& world)
    {
        const std::vector<uint16_t> MAPS_TO_CLEAR_ENTITIES_FROM = {
                MAP_MERCATOR_CASTLE_ENTRANCE_HALLWAY,
                MAP_MERCATOR_CASTLE_MAIN_HALL,
                MAP_ROUTE_GUMI_RYUMA_MERCATOR_GATES,
                MAP_GUMI_EXTERIOR,
                MAP_MERCATOR_CASTLE_BANQUET_ROOM,
                MAP_MECATOR_CASTLE_TOWER_SUMMIT,
                MAP_MERCATOR_CASTLE_ZAK_ROOM,
                MAP_MERCATOR_CASTLE_DEXTER_ROOM,
                MAP_MERCATOR_CASTLE_NIGEL_ROOM,
                MAP_MERCATOR_CASTLE_ARMORY_BARRACKS,
                MAP_RYUMA_LIGHTHOUSE, // Base Ryuma lighthouse (before Duke breaks it) is not reachable
                MAP_KN_CAVE_TELEPORTER_TO_KAZALT
        };

        for(uint16_t map_id : MAPS_TO_CLEAR_ENTITIES_FROM)
            world.map(map_id)->clear_entities();

        // Remove a useless Miro from a map in Swamp Shrine
        world.map(MAP_SWAMP_SHRINE_0)->remove_entity(7);
    }

    static void optimize_palettes_usage(World& world)
    {
        world.map(MAP_MERCATOR_EXTERIOR_2_VARIANT_2)->entity(0)->palette(1);

        world.map(MAP_VERLA_MINES_CHEST_ROOM_AFTER_LAVA_WALK)->palette(world.map(MAP_VERLA_MINES_MARLEY_LAVA_ROOM_MOVING_PLATFORMS)->palette());

        Map* gumi_happy = world.map(MAP_GUMI_EXTERIOR_VARIANT_HAPPY);
        gumi_happy->entity(0)->palette(1);
        gumi_happy->entity(1)->palette(1);
        gumi_happy->entity(4)->palette(1);
        gumi_happy->remove_entity(3);

        const std::vector<uint16_t> MAPS_TO_SWAP_PALETTE_3_AND_1 = {
                MAP_VERLA_MINES_JAR_STAIRCASE_RIDDLE_ROOM,
                MAP_VERLA_MINES_LIZARDS_LAVA_ROOM_MARLEY_SECTOR,
                MAP_DESTEL_WELL_WATERY_HUB,
                MAP_DESTEL_WELL_WATERY_ROOM_BEFORE_BOSS,
                MAP_MIR_TOWER_SECTOR_ROUTE_TO_TOWER_1,
                MAP_ROUTE_GUMI_RYUMA_BOULDER,
                MAP_MIR_TOWER_FALLING_SPIKEBALLS_ROOM,
                MAP_KN_LABYRINTH_ROOM_ABOVE_IRON_BOOTS,
                MAP_KN_LABYRINTH_SPIRAL_ROOM,
                MAP_KN_PALACE_JAR_AND_MOVING_PLATFORM_ROOM
        };

        for(uint16_t map_id : MAPS_TO_SWAP_PALETTE_3_AND_1)
        {
            Map* verla_jar_staircase = world.map(map_id);
            for(Entity* entity : verla_jar_staircase->entities())
            {
                if(entity->palette() == 3)
                    entity->palette(1);
            }
        }

        const std::vector<uint16_t> MAPS_TO_SWAP_PALETTE_3_AND_2 = {
                MAP_MIR_TOWER_EXTERIOR,
                MAP_CRYPT_MAIN_HALL
        };

        for(uint16_t map_id : MAPS_TO_SWAP_PALETTE_3_AND_2)
        {
            Map* verla_jar_staircase = world.map(map_id);
            for(Entity* entity : verla_jar_staircase->entities())
            {
                if(entity->palette() == 3)
                    entity->palette(2);
            }
        }
    }

    static void optimize_map_connections(World& world)
    {
        // Replace direct waterless variants references by parent map in Thieves Hideout
        world.map_connection(MAP_THIEVES_HIDEOUT_DOOR_BEHIND_WATERFALL_WATERLESS, MAP_THIEVES_HIDEOUT_BACK_ACCESS)
             .replace_map(MAP_THIEVES_HIDEOUT_DOOR_BEHIND_WATERFALL_WATERLESS, MAP_THIEVES_HIDEOUT_DOOR_BEHIND_WATERFALL);

        world.map_connection(MAP_THIEVES_HIDEOUT_ENTRANCE_WATERLESS, MAP_THIEVES_HIDEOUT_POCKETS_CELL_ACCESS_1)
             .replace_map(MAP_THIEVES_HIDEOUT_ENTRANCE_WATERLESS, MAP_THIEVES_HIDEOUT_ENTRANCE);

        // Replace direct solified magma variant reference by base version of Lake Shrine exterior map when leaving Lake Shrine
        // world.map_connection(MAP_LAKE_SHRINE_ENTRANCE, MAP_LAKE_SHRINE_EXTERIOR_WITH_MAGMA)
        //      .replace_map(MAP_LAKE_SHRINE_EXTERIOR_WITH_MAGMA, MAP_LAKE_SHRINE_EXTERIOR);
    }

    static void remove_useless_map_variants(World& world)
    {
        // Clear unreachable variants and prevent them from triggering
        const std::vector<uint16_t> MAPS_TO_REMOVE_VARIANTS_FROM = {
                MAP_THIEVES_HIDEOUT_TREASURE_ROOM,
                MAP_MERCATOR_CASTLE_THRONE_ROOM,
                MAP_MERCATOR_CASTLE_MAIN_HALL,
                MAP_MERCATOR_CASTLE_ENTRANCE_HALLWAY,
                MAP_MERCATOR_CASTLE_ARMORY_0F,
                MAP_MERCATOR_DOCKS_SUPPLY_SHOP
        };

        for(uint16_t map_id : MAPS_TO_REMOVE_VARIANTS_FROM)
        {
            Map* map = world.map(map_id);
            for(auto& [variant_map, flag] : map->variants())
                variant_map->clear();
            map->variants().clear();
        }
    }

    static void transform_variant_into_standard(Map* map)
    {
        Map* parent_map = map;
        while(parent_map->parent_map())
            parent_map = parent_map->parent_map();

        parent_map->clear_entities();
        for(Entity* entity : map->entities())
            parent_map->add_entity(new Entity(*entity));

        for(auto& [variant_map, flag] : parent_map->variants())
            variant_map->clear();
        parent_map->variants().clear();
    }
};
