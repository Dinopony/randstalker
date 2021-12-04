#include "kaizo.hpp"

#include <set>
#include <iostream>

void setup_mercator_castle_maps(World& world)
{
    const std::vector<uint16_t> maps_to_setup = {
        MAP_MERCATOR_DUNGEON_HALLWAY_AFTER_PRINCESS_ROOM, MAP_MERCATOR_CASTLE_PASSAGE_TO_DUNGEON,
        MAP_MERCATOR_CASTLE_BIG_LIBRARY, MAP_MERCATOR_CASTLE_SMALL_LIBRARY, MAP_MERCATOR_CASTLE_RIGHT_AISLE_BEDROOM, 
        MAP_MERCATOR_CASTLE_MAIN_HALL, MAP_MERCATOR_CASTLE_THRONE_ROOM, MAP_MERCATOR_CASTLE_ENTRANCE_HALLWAY,
        MAP_MERCATOR_CASTLE_1F_HALLWAY, MAP_MERCATOR_CASTLE_BANQUET_ROOM,
        MAP_MERCATOR_CASTLE_2F_HALLWAY, MAP_MERCATOR_CASTLE_DEXTER_ROOM, MAP_MERCATOR_CASTLE_ZAK_ROOM, MAP_MERCATOR_CASTLE_NIGEL_ROOM, 
        MAP_MERCATOR_CASTLE_KAYLA_ROOM, MAP_MERCATOR_CASTLE_KAYLA_BATHROOM_ENTRANCE, MAP_MERCATOR_CASTLE_KAYLA_BATHROOM,
        MAP_MERCATOR_CASTLE_ARMORY_0F, MAP_MERCATOR_CASTLE_ARMORY_1F, MAP_MERCATOR_CASTLE_ARMORY_2F, 
        MAP_MERCATOR_CASTLE_ARMORY_3F, MAP_MERCATOR_CASTLE_GUARDS_BEDROOM,
        MAP_MERCATOR_CASTLE_LEFT_CORNER_ROOM, MAP_MERCATOR_CASTLE_KITCHEN_AISLE_HALLWAY, 
        MAP_MERCATOR_CASTLE_KITCHEN_BEDROOM_1, MAP_MERCATOR_CASTLE_KITCHEN_BEDROOM_2,
        MAP_MERCATOR_CASTLE_KITCHEN, MAP_MERCATOR_CASTLE_KITCHEN_BACK_HALLWAY, 
        MAP_MERCATOR_CASTLE_EXTERIOR, MAP_MERCATOR_CASTLE_EXTERIOR_LEFT_COURT
    };

    const std::set<uint16_t> maps_keeping_entities = { 
        MAP_MERCATOR_CASTLE_PASSAGE_TO_DUNGEON,
        MAP_MERCATOR_CASTLE_KITCHEN,
        MAP_MERCATOR_CASTLE_ARMORY_2F
    };

    MapPalette* castle_palette = world.map(MAP_MERCATOR_CASTLE_MAIN_HALL)->palette();
    MapPalette* castle_palette_darkened = new MapPalette(*castle_palette);
    for(uint8_t i=0 ; i<4 ; ++i)
        castle_palette_darkened->color(i).apply_factor(0.55f);
    castle_palette_darkened->color(8).r--;
    castle_palette_darkened->color(9).r--;
    world.map_palettes().push_back(castle_palette_darkened);

    for(uint16_t map_id : maps_to_setup)
    {
        Map* map = world.map(map_id);
        wipe_map_variants(map);

        map->background_music(1); // Set dungeon music

        if(!maps_keeping_entities.count(map_id))
            map->clear_entities();

        if(map->palette() == castle_palette)
            map->palette(castle_palette_darkened);
    }
}

void edit_castle_passage_to_dungeon(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_PASSAGE_TO_DUNGEON);
    map->entity(0)->liftable(true);
    map->entity(1)->liftable(true);
    map->entity(2)->liftable(true);
    map->add_entity(new Entity(ENTITY_LARGE_GRAY_SPIKEBALL, 0x14, 0x18, 2))->position().half_x = true;
}

void edit_castle_big_library(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_BIG_LIBRARY);
    map->add_entity(new Entity(ENEMY_MUMMY_1, 0x19, 0x19, 1))->palette(0);
    map->add_entity(new Entity(ENEMY_MUMMY_1, 0x16, 0x19, 1))->palette(0);
    
    Entity* ekeeke = map->add_entity(new Entity(0xC0, 0x18, 0x13, 2));
    ekeeke->liftable(true);
    ekeeke->position().half_x = true;

    // TODO: Put proper base_chest_id on map
    map->add_entity(new Entity(ENTITY_CHEST, 0x1C, 0x1E, 2));

    Entity* gate = map->add_entity(new Entity(ENTITY_GATE_SOUTH, 0x19, 0x21, 1));
    gate->behavior_id(BEHAVIOR_DISAPPEAR_ON_ALL_ENEMIES_BEATEN);
    gate->palette(2);
    gate->appear_after_player_moved_away(true);
}

void edit_castle_small_library(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_SMALL_LIBRARY);

    map->add_entity(new Entity(ENEMY_SKELETON_1, 0x1C, 0x1E, 0))->palette(0);
    map->add_entity(new Entity(ENEMY_MUMMY_1, 0x16, 0x1C, 0))->palette(0);
    map->add_entity(new Entity(ENEMY_MUMMY_1, 0x13, 0x1D, 0))->palette(0);
    
    // TODO: Put proper base_chest_id on map
    map->add_entity(new Entity(ENTITY_CHEST, 0x1A, 0x1A, 1));
    map->add_entity(new Entity(ENTITY_CHEST, 0x16, 0x1A, 5))->orientation(ENTITY_ORIENTATION_NW);

    Entity* first_gate = map->add_entity(new Entity(ENTITY_GATE_NORTH, 0x17, 0x18, 0));
    first_gate->position().half_x = true;
    first_gate->behavior_id(BEHAVIOR_DISAPPEAR_ON_ALL_ENEMIES_BEATEN);
    first_gate->palette(2);

    Entity* gate = map->add_entity(new Entity(ENTITY_GATE_NORTH, 0x1B, 0x10, 5));
    gate->position().half_x = true;
    gate->entity_to_use_tiles_from(first_gate);
    gate->behavior_id(BEHAVIOR_DISAPPEAR_ON_ALL_ENEMIES_BEATEN);
    gate->palette(2);
}

void edit_castle_right_aisle_bedroom(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_RIGHT_AISLE_BEDROOM);

    Entity* vase = map->add_entity(new Entity(ENTITY_VASE, 0x18, 0x0E, 2));
    vase->position().half_x = true;
    vase->position().half_y = true;

    Entity* gate = map->add_entity(new Entity(ENTITY_GATE_SOUTH, 0x14, 0x1B, 1));
    gate->palette(2);
    gate->behavior_id(BEHAVIOR_ALLOW_APPEAR_AFTER_PLAYER_MOVED_AWAY);
    gate->appear_after_player_moved_away(true);

    Entity* skeleton = map->add_entity(new Entity(ENEMY_SKELETON_1, 0x12, 0x10, 1));
    skeleton->palette(0);
    skeleton->orientation(ENTITY_ORIENTATION_SW);

    skeleton = map->add_entity(new Entity(ENEMY_SKELETON_1, 0x18, 0x17, 1));
    skeleton->palette(0);
    skeleton->orientation(ENTITY_ORIENTATION_NE);
}

void edit_castle_1f_hallway(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_1F_HALLWAY);

    Entity* first_wall = map->add_entity(new Entity(ENTITY_MIR_REMOVABLE_WALL, 0x12, 0x1C, 0));
    first_wall->palette(0);
    first_wall->orientation(ENTITY_ORIENTATION_SE);

    for(uint8_t pos_y = 0x1D ; pos_y <= 0x21 ; ++pos_y)
    {
        Entity* wall = map->add_entity(new Entity(ENTITY_MIR_REMOVABLE_WALL, 0x12, pos_y, 0));
        wall->palette(0);
        wall->entity_to_use_tiles_from(first_wall);
        wall->orientation(ENTITY_ORIENTATION_SE);
    }

    Entity* gate = map->add_entity(new Entity(ENTITY_GATE_SOUTH, 0x23, 0x22, 0));
    gate->palette(2);
    gate->remove_when_flag_is_set(FLAG_ZAK_ROOM_BUTTON_PRESSED);

    map->add_entity(new Entity(ENTITY_CHEST, 0x0D, 0x1E, 0))->orientation(ENTITY_ORIENTATION_NW);
}

void edit_castle_2f_hallway(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_2F_HALLWAY);

    Entity* dexter_room_gate = map->add_entity(new Entity(ENTITY_GATE_SOUTH, 0x17, 0x14, 2));
    dexter_room_gate->palette(2);
    dexter_room_gate->orientation(ENTITY_ORIENTATION_NW);
    dexter_room_gate->remove_when_flag_is_set(FLAG_KAYLA_BATHROOM_BUTTON_PRESSED);

    Entity* zak_room_gate = map->add_entity(new Entity(ENTITY_GATE_SOUTH, 0x17, 0x21, 2));
    zak_room_gate->palette(2);
    zak_room_gate->entity_to_use_tiles_from(dexter_room_gate);
    zak_room_gate->orientation(ENTITY_ORIENTATION_NW);
    zak_room_gate->remove_when_flag_is_set(FLAG_DEXTER_ROOM_BUTTON_PRESSED);

    Entity* kayla_room_gate = map->add_entity(new Entity(ENTITY_GATE_NORTH, 0x0D, 0x1C, 2));
    kayla_room_gate->palette(2);
    kayla_room_gate->orientation(ENTITY_ORIENTATION_SE);
    kayla_room_gate->behavior_id(BEHAVIOR_DISAPPEAR_ON_ALL_ENEMIES_BEATEN);

    Entity* nigel_room_blocker = map->add_entity(new Entity(QUARTZ_SWORD_BLOCKER_TYPE, 0x0E, 0x25, 2));
    nigel_room_blocker->position().half_x = true;
    nigel_room_blocker->position().half_y = true;
    nigel_room_blocker->orientation(ENTITY_ORIENTATION_SE);

    Entity* nigel_room_blocker_anticheese = map->add_entity(new Entity(ENTITY_INVISIBLE_CUBE, 0x0E, 0x25, 5));
    nigel_room_blocker_anticheese->position().half_x = true;
    nigel_room_blocker_anticheese->position().half_y = true;

    Entity* mummy = map->add_entity(new Entity(ENEMY_MUMMY_1, 0x12, 0x1F, 2));
    mummy->palette(0);
    mummy->orientation(ENTITY_ORIENTATION_SE);
    
    mummy = map->add_entity(new Entity(ENEMY_MUMMY_1, 0x14, 0x28, 2));
    mummy->palette(0);
    mummy->orientation(ENTITY_ORIENTATION_NE);

    mummy = map->add_entity(new Entity(ENEMY_MUMMY_1, 0x15, 0x13, 2));
    mummy->palette(0);
    mummy->orientation(ENTITY_ORIENTATION_SW);
}

void edit_castle_kayla_room(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_KAYLA_ROOM);

    Entity* gate = map->add_entity(new Entity(ENTITY_GATE_SOUTH, 0x1B, 0x18, 0));
    gate->palette(2);
    gate->orientation(ENTITY_ORIENTATION_NW);
    gate->behavior_id(BEHAVIOR_DISAPPEAR_ON_ALL_ENEMIES_BEATEN);
    gate->appear_after_player_moved_away(true);

    gate = map->add_entity(new Entity(ENTITY_GATE_NORTH, 0x18, 0x0B, 0));
    gate->palette(2);
    gate->behavior_id(BEHAVIOR_DISAPPEAR_ON_ALL_ENEMIES_BEATEN);

    map->add_entity(new Entity(ENEMY_REAPER_1, 0x13, 0x14, 0))->palette(0);
    map->add_entity(new Entity(ENEMY_REAPER_1, 0x14, 0x14, 0))->palette(0);
    map->add_entity(new Entity(ENEMY_REAPER_1, 0x14, 0x15, 0))->palette(0);

    // On bathroom button pressed, remove all entities
    map->global_entity_mask_flags().push_back(GlobalEntityMaskFlag(FLAG_KAYLA_BATHROOM_BUTTON_PRESSED, 0));
}

void edit_castle_kayla_bathroom(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_KAYLA_BATHROOM);
    
    Entity* button = map->add_entity(new Entity(ENTITY_BUTTON, 0x0E, 0x0E, 2));
    button->behavior_id(BEHAVIOR_BUTTON_SET_PERSISTENCE_FLAG);
    button->persistence_flag(FLAG_KAYLA_BATHROOM_BUTTON_PRESSED);
}

void edit_castle_dexter_room(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_DEXTER_ROOM);

    Entity* first_spikeball = map->add_entity(new Entity(ENTITY_LARGE_GRAY_SPIKEBALL, 0x0F, 0x0F, 1));

    map->add_entity(new Entity(ENTITY_LARGE_GRAY_SPIKEBALL, 0x11, 0x12, 0))->entity_to_use_tiles_from(first_spikeball);
    map->add_entity(new Entity(ENTITY_LARGE_GRAY_SPIKEBALL, 0x0F, 0x0F, 1))->entity_to_use_tiles_from(first_spikeball);
    map->add_entity(new Entity(ENTITY_LARGE_GRAY_SPIKEBALL, 0x15, 0x16, 1))->entity_to_use_tiles_from(first_spikeball);
    map->add_entity(new Entity(ENTITY_LARGE_GRAY_SPIKEBALL, 0x0E, 0x17, 1))->entity_to_use_tiles_from(first_spikeball);
    map->add_entity(new Entity(ENTITY_LARGE_GRAY_SPIKEBALL, 0x14, 0x0E, 0))->entity_to_use_tiles_from(first_spikeball);
    map->add_entity(new Entity(ENTITY_LARGE_GRAY_SPIKEBALL, 0x15, 0x12, 1))->entity_to_use_tiles_from(first_spikeball);
    map->add_entity(new Entity(ENTITY_LARGE_GRAY_SPIKEBALL, 0x11, 0x18, 0))->entity_to_use_tiles_from(first_spikeball);

    map->add_entity(new Entity(ENTITY_CHEST, 0x0E, 0x0D, 5));
    map->add_entity(new Entity(ENTITY_CHEST, 0x18, 0x0E, 0));

    Entity* button = map->add_entity(new Entity(ENTITY_BUTTON, 0x10, 0x17, 0));
    button->position().half_x = true;
    button->position().half_y = true;
    button->behavior_id(BEHAVIOR_BUTTON_SET_PERSISTENCE_FLAG);
    button->persistence_flag(FLAG_DEXTER_ROOM_BUTTON_PRESSED);
}

void edit_castle_zak_room(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_ZAK_ROOM);

    Entity* gate = map->add_entity(new Entity(ENTITY_GATE_NORTH, 0x0B, 0x13, 0));
    gate->palette(2);
    gate->orientation(ENTITY_ORIENTATION_SE);

    Entity* first_platform = map->add_entity(new Entity(ENTITY_LARGE_DARK_PLATFORM, 0x17, 0x16, 1));
    make_entity_visible_with_golas_eye(first_platform);
    first_platform->palette(3);

    Entity* platform = map->add_entity(new Entity(ENTITY_LARGE_DARK_PLATFORM, 0x17, 0x18, 2));
    make_entity_visible_with_golas_eye(platform);
    platform->entity_to_use_tiles_from(first_platform);
    platform->palette(3);

    platform = map->add_entity(new Entity(ENTITY_LARGE_DARK_PLATFORM, 0x15, 0x18, 3));
    make_entity_visible_with_golas_eye(platform);
    platform->entity_to_use_tiles_from(first_platform);
    platform->palette(3);

    platform = map->add_entity(new Entity(ENTITY_LARGE_DARK_PLATFORM, 0x13, 0x18, 4));
    make_entity_visible_with_golas_eye(platform);
    platform->entity_to_use_tiles_from(first_platform);
    platform->palette(3);

    platform = map->add_entity(new Entity(ENTITY_LARGE_DARK_PLATFORM, 0x11, 0x18, 5));
    make_entity_visible_with_golas_eye(platform);
    platform->entity_to_use_tiles_from(first_platform);
    platform->palette(3);

    platform = map->add_entity(new Entity(ENTITY_LARGE_DARK_PLATFORM, 0x0F, 0x18, 6));
    platform->entity_to_use_tiles_from(first_platform);

    Entity* button = map->add_entity(new Entity(ENTITY_BUTTON, 0x0F, 0x18, 7));
    button->position().half_x = true;
    button->position().half_y = true;
    button->behavior_id(BEHAVIOR_BUTTON_SET_PERSISTENCE_FLAG);
    button->persistence_flag(FLAG_ZAK_ROOM_BUTTON_PRESSED);

    map->add_entity(new Entity(ENTITY_CRATE, 0x17, 0x18, 0));
}

void edit_castle_upper_main_hall(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_MAIN_HALL);

    Entity* first_concrete = map->add_entity(new Entity(ENTITY_LARGE_CONCRETE_BLOCK, 0x12, 0x16, 6));
    map->add_entity(new Entity(ENTITY_LARGE_CONCRETE_BLOCK, 0x2C, 0x16, 6))->entity_to_use_tiles_from(first_concrete);

    Entity* skeleton = map->add_entity(new Entity(ENEMY_SKELETON_1, 0x22, 0x14, 6));
    skeleton->palette(0);
    skeleton->orientation(ENTITY_ORIENTATION_SE);

    skeleton = map->add_entity(new Entity(ENEMY_SKELETON_1, 0x1D, 0x13, 6));
    skeleton->palette(0);
    skeleton->orientation(ENTITY_ORIENTATION_SE);

    skeleton = map->add_entity(new Entity(ENEMY_SKELETON_1, 0x15, 0x15, 6));
    skeleton->palette(0);
    skeleton->orientation(ENTITY_ORIENTATION_SE);
}

void edit_castle_left_court(World& world, md::ROM& rom)
{
    0x20D;
}

void edit_mercator_castle(World& world, md::ROM& rom)
{
    setup_mercator_castle_maps(world);
    edit_castle_passage_to_dungeon(world);
    edit_castle_big_library(world);
    edit_castle_small_library(world);
    edit_castle_right_aisle_bedroom(world);
    edit_castle_1f_hallway(world);
    edit_castle_2f_hallway(world);
    edit_castle_kayla_room(world);
    edit_castle_kayla_bathroom(world);
    edit_castle_dexter_room(world);
    edit_castle_zak_room(world);
    edit_castle_upper_main_hall(world);  
}