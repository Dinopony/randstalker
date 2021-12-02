#include "kaizo.hpp"

#include <iostream>

void setup_mercator_maps(World& world)
{
    const std::vector<uint16_t> maps_to_empty = {
        MAP_MERCATOR_EXTERIOR_1, MAP_MERCATOR_EXTERIOR_2, MAP_MERCATOR_FOUNTAIN,
        MAP_MERCATOR_MENAGERIE, MAP_MERCATOR_COUPLE_HOUSE, MAP_MERCATOR_CELLO_HOUSE, MAP_MERCATOR_SHOP,
        MAP_MERCATOR_MADAME_YARD_ENTRANCE, MAP_MERCATOR_MADAME_YARD_HALLWAY, MAP_MERCATOR_MADAME_YARD_ROOM_1, MAP_MERCATOR_MADAME_YARD_ROOM_2,
        MAP_MERCATOR_HOTEL,
        MAP_MERCATOR_BALCONY_INN_FIRST_FLOOR, MAP_MERCATOR_BALCONY_INN_SECOND_FLOOR,
        MAP_MERCATOR_HOUSE_NEAR_WELL_FIRST_FLOOR, MAP_MERCATOR_HOUSE_NEAR_WELL_SECOND_FLOOR,
        MAP_MERCATOR_CASTLE_BACKDOOR_COURT
    };

    MapPalette* exterior_palette = world.map(MAP_MERCATOR_EXTERIOR_1)->palette();
    MapPalette* exterior_palette_altered = new MapPalette(*exterior_palette);
    for(uint8_t i=4 ; i<6 ; ++i)
    {
        // Make greens more yellow-ish
        exterior_palette_altered->color(i).desaturate(0.3f);
        exterior_palette_altered->color(i).r += 9;
    }
    for(uint8_t i=9 ; i<11 ; ++i)
    {
        // Make blue (water) dark-greenish
        exterior_palette_altered->color(i).desaturate(1.0f);
        exterior_palette_altered->color(i).apply_factor(0.4f);
        exterior_palette_altered->color(i).g += 4;
    }
    // Darken the whole palette slightly
    exterior_palette_altered->apply_factor(0.9f);

    world.map_palettes().push_back(exterior_palette_altered);
    world.map(MAP_MERCATOR_EXTERIOR_1)->palette(exterior_palette_altered);
    world.map(MAP_MERCATOR_EXTERIOR_2)->palette(exterior_palette_altered);
    world.map(MAP_MERCATOR_FOUNTAIN)->palette(exterior_palette_altered);
    world.map(MAP_MERCATOR_CASTLE_BACKDOOR_COURT)->palette(exterior_palette_altered);

    for(uint16_t map_id : maps_to_empty)
    {
        Map* map = world.map(map_id);
        map->clear_entities();
        map->background_music(12); // Set sad music

        std::map<Map*, Flag>& variants = map->variants();
        for(auto& [variant_map, flag] : variants)
        {
            std::cout << "Freeing map #" << variant_map->id() << std::endl;
            variant_map->clear();
        }
        variants.clear();
    }
}

void edit_mercator_map_1(World& world)
{
    Map* map = world.map(MAP_MERCATOR_EXTERIOR_1);

    map->add_entity(new Entity(ENTITY_INJURED_DOG, 0x28, 0x2B, 1));
    auto soldier = map->add_entity(new Entity(ENTITY_NPC_SOLDIER, 0x28, 0x2A, 1));
    soldier->orientation(ENTITY_ORIENTATION_SW);
    soldier->talkable(true);
    world.game_strings()[0x37E] = "\u001cThis is a disaster...\nWe got raided by monsters,\nand nothing is left from\u001e\nour once glorious town...\nYou don't seem to understand,\nI'm the only one still alive!\u001e";
    world.game_strings()[0x37F] = "\u001cI've seen all of my\nfriends fall one by one...\u001e\nI'm gonna make them\nPAY FOR THIS!\u0003";

    map->add_entity(new Entity(ENTITY_INJURED_DOG, 0x23, 0x39, 1));
    
    const std::vector<std::pair<uint8_t, uint8_t>> boulder_coordinates = {
        { 0x22, 0x28 }, { 0x26, 0x24 }, { 0x16, 0x2C }, { 0x15, 0x2A }, { 0x17, 0x28 }
    };
    Entity* first_boulder = nullptr;
    for(auto& coords : boulder_coordinates)
    {
        Entity* boulder = map->add_entity(new Entity(ENTITY_ROCK, coords.first, coords.second, 1));
        boulder->palette(0);

        if(first_boulder)
            boulder->entity_to_use_tiles_from(first_boulder);
        else
            first_boulder = boulder;  
    }

    map->add_entity(new Entity(ENTITY_DEAD_BODY, 0x25, 0x25, 1))->orientation(ENTITY_ORIENTATION_NW);
    map->add_entity(new Entity(ENTITY_DEAD_BODY, 0x22, 0x35, 1));

    map->add_entity(new Entity(ENEMY_SKELETON_1, 0x1B, 0x37, 1));
    map->add_entity(new Entity(ENEMY_SKELETON_1, 0x13, 0x21, 1));
}

void edit_mercator_menagerie(World& world)
{
    Map* map = world.map(MAP_MERCATOR_MENAGERIE);

    map->add_entity(new Entity(ENTITY_CHEST, 0x16, 0x10, 4))->orientation(ENTITY_ORIENTATION_NW);
    map->base_chest_id(0x00); // TODO: Set proper chest ID!

    map->add_entity(new Entity(ENEMY_SKELETON_1, 0x14, 0x18, 2))->orientation(ENTITY_ORIENTATION_NW);
    map->add_entity(new Entity(ENEMY_SKELETON_1, 0x18, 0x14, 2))->orientation(ENTITY_ORIENTATION_SW);
}

void edit_madame_yard_building(World& world)
{
    Map* entrance_map = world.map(MAP_MERCATOR_MADAME_YARD_ENTRANCE);

    entrance_map->add_entity(new Entity(ENTITY_CHEST, 0x14, 0x12, 0))->orientation(ENTITY_ORIENTATION_NW);

    ////////////////////////////////
    Map* hallway_map = world.map(MAP_MERCATOR_MADAME_YARD_HALLWAY);
    
    Entity* miniaturizer_blocker = hallway_map->add_entity(new Entity(ENTITY_SMALL_GRAY_SPIKEBALL, 0x12, 0x14, 2));
    miniaturizer_blocker->half_tile_z(true);
    miniaturizer_blocker->palette(2);

    // TODO: Could also be miniaturizer if implemented

    ////////////////////////////////
    Map* room_1_map = world.map(MAP_MERCATOR_MADAME_YARD_ROOM_1);
    
    room_1_map->add_entity(new Entity(ENTITY_CHEST, 0x14, 0x10, 1));

    ////////////////////////////////
    Map* room_2_map = world.map(MAP_MERCATOR_MADAME_YARD_ROOM_2);
    
    // TODO: Once quartz sword is obtained
}

void edit_mercator_map_2(World& world)
{
    Map* map = world.map(MAP_MERCATOR_EXTERIOR_2);

    Entity* first_ball = map->add_entity(new Entity(POWER_GLOVE_BLOCKER_TYPE, 0x35, 0x21, 1));
    first_ball->half_tile_x(true);
    Entity* second_ball = map->add_entity(new Entity(POWER_GLOVE_BLOCKER_TYPE, 0x2C, 0x3A, 1));
    second_ball->half_tile_y(true);
    second_ball->entity_to_use_tiles_from(first_ball);

    const std::vector<std::pair<uint8_t, uint8_t>> boulder_coordinates = {
        { 0x2B, 0x29 }, { 0x30, 0x26 }, { 0x36, 0x2D }, 
        { 0x25, 0x36 }, { 0x14, 0x3B }, { 0x15, 0x28 }
    };

    Entity* first_boulder = nullptr;
    for(auto& coords : boulder_coordinates)
    {
        Entity* boulder = map->add_entity(new Entity(ENTITY_ROCK, coords.first, coords.second, 1));
        boulder->palette(0);

        if(first_boulder)
            boulder->entity_to_use_tiles_from(first_boulder);
        else
            first_boulder = boulder;
    }

    Entity* boulder = map->add_entity(new Entity(ENTITY_ROCK, 0x15, 0x21, 1));
    boulder->palette(0);
    boulder->half_tile_z(true);
    boulder->entity_to_use_tiles_from(first_boulder);

    map->add_entity(new Entity(ENTITY_SMALL_YELLOW_PLATFORM, 0x14, 0x21, 1));

    Entity* first_gate = map->add_entity(new Entity(ENTITY_GATE_NORTH, 0x25, 0x0C, 1));
    map->add_entity(new Entity(ENTITY_GATE_NORTH, 0x27, 0x0C, 1))->entity_to_use_tiles_from(first_gate);
}

void open_castle_backdoor(md::ROM& rom)
{
    // Set the castle backdoor to be always opened (checks flag 0 bit 0 instead of flag 7 bit 6)
    // 0716 ---> 0010
    rom.set_word(0x50DE, 0x0010);
}

void change_castle_backdoor_connections(World& world)
{
    world.swap_map_connections(MAP_MERCATOR_CASTLE_BACKDOOR_COURT, MAP_MERCATOR_CASTLE_BACKDOOR_BUTTON_ROOM,
                                MAP_MERCATOR_DUNGEON_PRINCESS_ROOM, MAP_MERCATOR_DUNGEON_HALLWAY_AFTER_PRINCESS_ROOM);
    
    world.swap_map_connections(MAP_MERCATOR_DUNGEON_HALLWAY_AFTER_PRINCESS_ROOM, MAP_MERCATOR_DUNGEON_LAST_HALLWAY_TO_TOWER_EXTERIOR,
                                MAP_MERCATOR_DUNGEON_STAIRWAY_ACCESS_TO_CASTLE, MAP_MERCATOR_CASTLE_PASSAGE_TO_DUNGEON);
}

void edit_mercator(World& world, md::ROM& rom)
{
    setup_mercator_maps(world);
    edit_mercator_map_1(world);
    edit_mercator_menagerie(world);
    edit_madame_yard_building(world);
    edit_mercator_map_2(world);
    open_castle_backdoor(rom);
    change_castle_backdoor_connections(world);
}
