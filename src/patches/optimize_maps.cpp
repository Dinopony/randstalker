#include <landstalker_lib/model/world.hpp>
#include <landstalker_lib/model/map.hpp>
#include <landstalker_lib/model/entity.hpp>
#include <landstalker_lib/constants/map_codes.hpp>

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

//////////////////////////////////////////////////////////////////////

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
            MAP_INTRO_139,
            MAP_INTRO_140,
            MAP_INTRO_141,
            MAP_INTRO_142,
            MAP_INTRO_143,

            MAP_MERCATOR_CASTLE_KAYLA_ROOM,
            MAP_MERCATOR_CASTLE_KAYLA_BATHROOM_ENTRANCE,
            MAP_MERCATOR_CASTLE_KAYLA_BATHROOM
    };

    for(uint16_t map_id : UNREACHABLE_MAPS)
        world.map(map_id)->clear();
}

static void remove_useless_entities(World& world)
{
    world.map(MAP_MERCATOR_CASTLE_ENTRANCE_HALLWAY)->clear_entities();
    world.map(MAP_MERCATOR_CASTLE_MAIN_HALL)->clear_entities();
    world.map(MAP_ROUTE_GUMI_RYUMA_MERCATOR_GATES)->clear_entities();
    world.map(MAP_GUMI_EXTERIOR)->clear_entities();

    // Remove a useless Miro from a map in Swamp Shrine
    world.map(MAP_SWAMP_SHRINE_0)->remove_entity(7);
}

static void optimize_palettes(World& world)
{
    world.map(MAP_MERCATOR_EXTERIOR_2_VARIANT_2)->entity(0)->palette(1);

    Map* gumi_happy = world.map(MAP_GUMI_EXTERIOR_VARIANT_HAPPY);
    gumi_happy->entity(0)->palette(1);
    gumi_happy->entity(1)->palette(1);
    gumi_happy->entity(4)->palette(1);
    gumi_happy->remove_entity(3);
}

static void remove_useless_map_variants(World& world)
{
    // Clear unreachable variants and prevent them from triggering
    const std::vector<uint16_t> MAPS_TO_REMOVE_VARIANTS_FROM = {
            MAP_THIEVES_HIDEOUT_TREASURE_ROOM,
            MAP_MERCATOR_CASTLE_THRONE_ROOM,
            MAP_MERCATOR_CASTLE_MAIN_HALL,
            MAP_MERCATOR_CASTLE_ENTRANCE_HALLWAY
    };

    for(uint16_t map_id : MAPS_TO_REMOVE_VARIANTS_FROM)
    {
        Map* map = world.map(map_id);
        for(auto& [variant_map, flag] : map->variants())
            variant_map->clear();
        map->variants().clear();
    }
}

void optimize_maps(World& world)
{
    optimize_laggy_tibor_rooms(world);
    clear_unreachable_maps(world);
    optimize_palettes(world);

    transform_variant_into_standard(world.map(MAP_MERCATOR_CASTLE_THRONE_ROOM_ARTHUR_VARIANT));

    remove_useless_map_variants(world);
}