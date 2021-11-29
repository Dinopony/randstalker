#include "../world.hpp"

#include "../tools/megadrive/rom.hpp"
#include "../tools/megadrive/code.hpp"

#include "../randomizer_options.hpp"
#include "../model/map.hpp"
#include "../model/entity.hpp"
#include "../model/entity_type.hpp"

#include "../constants/entity_type_codes.hpp"
#include "../constants/map_codes.hpp"
#include "../constants/item_codes.hpp"

#include <iostream>
#include <set>

std::vector<uint16_t> available_map_ids = {};

constexpr uint8_t POWER_GLOVE_BLOCKER_TYPE = ENTITY_LARGE_BLUE_BOULDER;
constexpr uint8_t QUARTZ_SWORD_BLOCKER_TYPE = ENTITY_GOLD_GOLEM_STATUE;

constexpr uint16_t BEHAVIOR_DISAPPEAR_ON_ALL_ENEMIES_BEATEN = 106;
constexpr uint16_t BEHAVIOR_ALLOW_APPEAR_AFTER_PLAYER_MOVED_AWAY = 785;

void add_boulder_blocking_thieves_hideout(World& world)
{
    Map* map = world.map(MAP_RYUMA_EXTERIOR_VARIANT);
    map->add_entity(new Entity(POWER_GLOVE_BLOCKER_TYPE, 0x35, 0x1E, 1));
    map->remove_entity(4); // Remove dog to prevent cheesing boulder by jumping over
}

void edit_kado_climb_map(World& world)
{
    Map* map = world.map(MAP_ROUTE_GUMI_RYUMA_KADO_CLIMB);
    Entity* chest = map->entity(8);
    chest->entity_type_id(ENTITY_CRATE); // Turn chest into a crate
    chest->liftable(true);
    map->entity(4)->half_tile_y(true); // Move slightly a platform towards SW to make a jump possible
    map->entity(5)->half_tile_z(true); // Move slightly a platform up to make a jump possible
    map->entity(9)->pos_z(7); // Raise a platform to serve as a place where to put the crate
    map->remove_entity(13); // Remove the spikeball
    map->remove_entity(12); // Remove the final platform
    map->remove_entity(10); // Remove the platform right before the chest
    map->remove_entity(6); // Remove the thin platform moving around
    map->remove_entity(3); // Remove an intermediate platform
    map->remove_entity(2); // Remove an intermediate platform
}

void edit_kado_textlines(World& world)
{
    world.game_strings()[0x736] = "\x1cMy name is Kado.\nI lost my Safety Pass\non the route yesterday.\x1e";
    world.game_strings()[0x737] = "\x1cI think witch Helga would\nbe able to locate it magically,\nbut I'm too old to go there.\x1e";
    world.game_strings()[0x738] = "\x1cYou can find her in the\nswamp not far from Mercator,\nnorth from here.\x1e";
    world.game_strings()[0x739] = "\x1cThis will help you against\nthe dangers of this world.";
}

void edit_helga_swamp_map(World& world)
{
    Map* map = world.map(MAP_ROUTE_GUMI_RYUMA_HELGA_SWAMP);
    map->clear_entities();
    map->add_entity(new Entity(ENTITY_SMALL_GRAY_SPIKEBALL, 0x16, 0x27, 4));
    map->add_entity(new Entity(ENTITY_SMALL_YELLOW_PLATFORM, 0x17, 0x27, 4))->half_tile_x(true);

    map->add_entity(new Entity(ENTITY_SMALL_GRAY_SPIKEBALL, 0x1B, 0x27, 4));
    map->add_entity(new Entity(ENTITY_SMALL_YELLOW_PLATFORM, 0x1B, 0x25, 4))->half_tile_y(true);

    map->add_entity(new Entity(ENTITY_SMALL_YELLOW_PLATFORM, 0x1B, 0x21, 4));
    map->add_entity(new Entity(ENTITY_SMALL_GRAY_SPIKEBALL, 0x1B, 0x1F, 4))->half_tile_y(true);

    map->add_entity(new Entity(ENTITY_SMALL_GRAY_SPIKEBALL, 0x18, 0x1F, 4));
    map->add_entity(new Entity(ENTITY_SMALL_YELLOW_PLATFORM, 0x18, 0x1C, 4));

    map->add_entity(new Entity(ENTITY_SMALL_GRAY_SPIKEBALL, 0x19, 0x15, 5));
    auto moving_platform = map->add_entity(new Entity(ENTITY_SMALL_YELLOW_PLATFORM, 0x19, 0x1B, 4));
    moving_platform->behavior_id(451);
    moving_platform->speed(2);
    moving_platform->gravity_immune(true);

    map->add_entity(new Entity(ENTITY_LARGE_GRAY_SPIKEBALL, 0x1B, 0x11, 1))->palette(2);
    map->add_entity(new Entity(ENTITY_LARGE_THIN_YELLOW_PLATFORM, 0x1B, 0x11, 4));
    map->add_entity(new Entity(ENTITY_LARGE_THIN_YELLOW_PLATFORM, 0x1B, 0x13, 4));
    map->add_entity(new Entity(ENTITY_LARGE_THIN_YELLOW_PLATFORM, 0x19, 0x13, 4));

    map->add_entity(new Entity(ENTITY_CHEST, 0x12, 0x15, 5));
    map->base_chest_id(134); // Use the chest ID of removed Kado climb chest
}

void edit_helga_swamp_map_2(World& world)
{
    Map* map = world.map(MAP_ROUTE_GUMI_RYUMA_HELGA_SWAMP_2);
    map->entity(1)->speed(4);

    map->add_entity(new Entity(ENTITY_LARGE_THIN_YELLOW_PLATFORM, 0x22, 0x29, 4));
    map->add_entity(new Entity(ENTITY_LARGE_GRAY_SPIKEBALL, 0x22, 0x28, 1))->palette(2);
    map->add_entity(new Entity(ENTITY_LARGE_THIN_YELLOW_PLATFORM, 0x22, 0x27, 4));
    map->add_entity(new Entity(ENTITY_SMALL_YELLOW_PLATFORM, 0x25, 0x27, 2));
    map->add_entity(new Entity(ENTITY_SMALL_YELLOW_PLATFORM, 0x25, 0x25, 0))->half_tile_z(true);

    map->add_entity(new Entity(ENTITY_SACRED_TREE, 0x19, 0x23, 5))->half_tile_x(true);
    map->add_entity(new Entity(ENTITY_SACRED_TREE, 0x1A, 0x23, 5))->half_tile_x(true);

    map->add_entity(new Entity(ENTITY_SMALL_THIN_YELLOW_PLATFORM, 0x1F, 0x1A, 4));
    map->add_entity(new Entity(ENTITY_SMALL_THIN_YELLOW_PLATFORM, 0x22, 0x1A, 6));

    map->add_entity(new Entity(ENTITY_CHEST, 0x19, 0x1A, 7))->orientation(ENTITY_ORIENTATION_NE);
    map->base_chest_id(0x00); // TODO! Use an empty chest ID

    map->remove_entity(0);
}

void edit_alter_helga_dialogue(World& world, md::ROM& rom)
{
    world.game_strings()[0x497] = "\x1cYou want to find a lost\nitem... Let me take a look...\nOh... I can feel it...\x03"
                                  "\nWhat you are looking for\nhas been eaten by a blue worm.\nIt's not far away from here.\x03"
                                  "\nLook for that blue worm and\nyou might find your pass\nto Mercator city.\x03"
                                  "\nIf you go there, say hello\nto my sister Linda. She works\nthere as a fortune teller.\x03";

    world.game_strings()[0x498] = "\x1cHave you found that worm?\nI can feel it not that far away.\nBut it seems powerful...\x03";

    md::Code helga_dialogue_script;
    helga_dialogue_script.btst(4, addr_(0xFF1004));
    helga_dialogue_script.beq(3);
        helga_dialogue_script.trap(0x1, { 0x00, 0x21 }); // Show reminder cutscene
        helga_dialogue_script.rts();
    helga_dialogue_script.btst(7, addr_(0xFF101B));
    helga_dialogue_script.beq(4);
        helga_dialogue_script.bset(4, addr_(0xFF1004));
        helga_dialogue_script.trap(0x1, { 0x00, 0x0D }); // Show "locating Safety Pass" cutscene
        helga_dialogue_script.rts();
    helga_dialogue_script.trap(0x1, { 0x00, 0x09 }); // Show useless chitchat as long as Kado has not been talked to
    helga_dialogue_script.rts();

    // Reminder cutscene
    helga_dialogue_script.add_bytes({
        0xE4, 0x4B // Print message 0x498 and close the textbox
    });
    // "Locating Safety Pass" cutscene
    helga_dialogue_script.add_bytes({
        0xE4, 0x4A // Print message 0x497 and close the textbox
    });
    // Useless chitchat as long as Kado has not been talked to
    helga_dialogue_script.add_bytes({
        0xE4, 0x75 // Print message 0x4C2 and close the textbox
    });

    uint32_t addr = rom.inject_code(helga_dialogue_script);
    rom.set_code(0x2731A, md::Code().jmp(addr));
}

void add_ultra_worm_to_gumi_boulder_map(World& world)
{
    Map* map = world.map(MAP_ROUTE_GUMI_RYUMA_BOULDER);
    auto ultra_worm = map->add_entity(new Entity(ENEMY_WORM_3, 0x18, 0x1D, 1));
    ultra_worm->mask_flags().push_back(EntityMaskFlag(true, 4, 4)); // Show ultra worm only if bit 4 of flag 1004 is set
    ultra_worm->mask_flags().push_back(EntityMaskFlag(false, 0x59, 5)); // Hide ultra worm if player has Safety Pass
    ultra_worm->speed(2);
    ultra_worm->behavior_id(1);

    // Change blue worm stats
    EntityEnemy* ultra_worm_stats = reinterpret_cast<EntityEnemy*>(world.entity_type(ENEMY_WORM_3));
    ultra_worm_stats->dropped_item(world.item(ITEM_SAFETY_PASS));
    ultra_worm_stats->drop_probability(1);
    ultra_worm_stats->health(150);

    // Make the platform replacing the boulder slightly higher to prevent caging the snake in the boulder hole
    map->entity(6)->half_tile_z(true);
}

void empty_mercator_maps(World& world)
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

    for(uint16_t map_id : maps_to_empty)
    {
        Map* map = world.map(map_id);
        map->clear_entities();
        map->background_music(12); // Set sad music

        std::map<Map*, Flag>& variants = map->variants();
        for(auto& [variant_map, flag] : variants)
        {
            available_map_ids.push_back(variant_map->id());
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

    for(uint16_t map_id : maps_to_setup)
    {
        Map* map = world.map(map_id);
        map->background_music(1); // Set dungeon music

        if(!maps_keeping_entities.count(map_id))
            map->clear_entities();

        std::map<Map*, Flag>& variants = map->variants();
        for(auto& [variant_map, flag] : variants)
        {
            available_map_ids.push_back(variant_map->id());
            variant_map->clear();
        }
        variants.clear();
    }
}

void edit_castle_passage_to_dungeon(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_PASSAGE_TO_DUNGEON);
    map->entity(0)->liftable(true);
    map->entity(1)->liftable(true);
    map->entity(2)->liftable(true);
    map->add_entity(new Entity(ENTITY_LARGE_GRAY_SPIKEBALL, 0x14, 0x18, 2))->half_tile_x(true);
}

void edit_castle_big_library(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_BIG_LIBRARY);
    map->add_entity(new Entity(ENEMY_MUMMY_1, 0x19, 0x19, 1))->palette(0);
    map->add_entity(new Entity(ENEMY_MUMMY_1, 0x16, 0x19, 1))->palette(0);
    
    Entity* ekeeke = map->add_entity(new Entity(0xC0, 0x18, 0x13, 2));
    ekeeke->liftable(true);
    ekeeke->half_tile_x(true);

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
    first_gate->half_tile_x(true);
    first_gate->behavior_id(BEHAVIOR_DISAPPEAR_ON_ALL_ENEMIES_BEATEN);
    first_gate->palette(2);

    Entity* gate = map->add_entity(new Entity(ENTITY_GATE_NORTH, 0x1B, 0x10, 5));
    gate->half_tile_x(true);
    gate->entity_to_use_tiles_from(first_gate);
    gate->behavior_id(BEHAVIOR_DISAPPEAR_ON_ALL_ENEMIES_BEATEN);
    gate->palette(2);
}

void edit_castle_right_aisle_bedroom(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_RIGHT_AISLE_BEDROOM);

    Entity* vase = map->add_entity(new Entity(ENTITY_VASE, 0x18, 0x0E, 2));
    vase->half_tile_x(true);
    vase->half_tile_y(true);

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
}

void edit_castle_2f_hallway(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_2F_HALLWAY);

    Entity* dexter_room_gate = map->add_entity(new Entity(ENTITY_GATE_SOUTH, 0x17, 0x14, 2));
    dexter_room_gate->palette(2);
    dexter_room_gate->orientation(ENTITY_ORIENTATION_NW);

    Entity* zak_room_gate = map->add_entity(new Entity(ENTITY_GATE_SOUTH, 0x17, 0x21, 2));
    zak_room_gate->palette(2);
    zak_room_gate->entity_to_use_tiles_from(dexter_room_gate);
    zak_room_gate->orientation(ENTITY_ORIENTATION_NW);

    Entity* kayla_room_gate = map->add_entity(new Entity(ENTITY_GATE_NORTH, 0x0D, 0x1C, 2));
    kayla_room_gate->palette(2);
    kayla_room_gate->orientation(ENTITY_ORIENTATION_SE);
    kayla_room_gate->behavior_id(BEHAVIOR_DISAPPEAR_ON_ALL_ENEMIES_BEATEN);

    Entity* nigel_room_blocker = map->add_entity(new Entity(QUARTZ_SWORD_BLOCKER_TYPE, 0x0E, 0x25, 2));
    nigel_room_blocker->half_tile_x(true);
    nigel_room_blocker->half_tile_y(true);
    nigel_room_blocker->orientation(ENTITY_ORIENTATION_SE);

    Entity* nigel_room_blocker_anticheese = map->add_entity(new Entity(ENTITY_INVISIBLE_CUBE, 0x0E, 0x25, 5));
    nigel_room_blocker_anticheese->half_tile_x(true);
    nigel_room_blocker_anticheese->half_tile_y(true);

    Entity* mummy = map->add_entity(new Entity(ENEMY_MUMMY_1, 0x12, 0x1F, 2));
    mummy->palette(0);
    mummy->orientation(ENTITY_ORIENTATION_SE);
    
    mummy = map->add_entity(new Entity(ENEMY_MUMMY_1, 0x14, 0x28, 2));
    mummy->palette(0);
    mummy->orientation(ENTITY_ORIENTATION_NE);
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
}

void edit_castle_kayla_bathroom(World& world)
{
    Map* map = world.map(MAP_MERCATOR_CASTLE_KAYLA_BATHROOM);
    
    Entity* button = map->add_entity(new Entity(ENTITY_BUTTON, 0x0E, 0x0E, 2));
    button->behavior_id(533);
}

void apply_kaizo_edits(World& world, md::ROM& rom)
{
    add_boulder_blocking_thieves_hideout(world);
    edit_kado_climb_map(world);
    edit_kado_textlines(world);
    edit_helga_swamp_map(world);
    edit_helga_swamp_map_2(world);
    edit_alter_helga_dialogue(world, rom);
    add_ultra_worm_to_gumi_boulder_map(world);

    empty_mercator_maps(world);
    edit_mercator_map_1(world);
    edit_mercator_menagerie(world);
    edit_mercator_map_2(world);
    open_castle_backdoor(rom);
    change_castle_backdoor_connections(world);

    setup_mercator_castle_maps(world);
    edit_castle_passage_to_dungeon(world);
    edit_castle_big_library(world);
    edit_castle_small_library(world);
    edit_castle_right_aisle_bedroom(world);
    edit_castle_1f_hallway(world);
    edit_castle_2f_hallway(world);
    edit_castle_kayla_room(world);
    edit_castle_kayla_bathroom(world);

    std::cout << "Free map IDs:\n";
    for(uint16_t free_map_id : available_map_ids)
        std::cout << "- " << free_map_id << "\n";
    std::cout << std::endl;
}
