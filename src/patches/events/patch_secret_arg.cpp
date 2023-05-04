#include "patch_secret_arg.hpp"

#include <landstalker-lib/model/world.hpp>
#include <landstalker-lib/model/map.hpp>
#include <landstalker-lib/model/entity.hpp>
#include <landstalker-lib/model/item.hpp>
#include <landstalker-lib/tools/game_text.hpp>
#include <landstalker-lib/constants/entity_type_codes.hpp>
#include <landstalker-lib/constants/map_codes.hpp>
#include <landstalker-lib/constants/item_codes.hpp>
#include <landstalker-lib/constants/offsets.hpp>
#include "../../assets/secret_arg_music_bank.bin.hxx"
#include "../../constants/rando_constants.hpp"
#include "../../logic_model/randomizer_world.hpp"

// =========================== GNOME HANDLING ===========================

constexpr uint8_t GNOME_TEXT = 0x1A; // Replaces Ludwig
static ByteArray gnome_text_map_ids_table;
static ByteArray gnome_command_words_table;

static uint32_t inject_func_gnome_text(md::ROM& rom, uint32_t gnome_map_ids_addr, uint32_t gnome_command_words_block)
{
    md::Code func;
    func.movem_to_stack({ reg_D0, reg_D1, reg_D2 }, { reg_A0 });
    {
        func.movew(addr_(0xFF1206), reg_D2);
        func.movew(0x0001, reg_D0);
        func.lea(gnome_map_ids_addr, reg_A0);
        func.label("loop_start");
        {
            func.movew(addr_(reg_A0), reg_D1);
            func.bmi("return"); // D1 = 0xFFFF, reached end of list without finding the appropriate map

            func.cmpw(reg_D1, reg_D2);
            func.bne("map_not_found");
            {
                // Right map found
                func.lea(gnome_command_words_block, reg_A0);
                func.jsr(0x253F8); // RunTextCmd function
                func.trap(0, { 0x00, 0x74 });
                func.bra("return");
            }
            func.label("map_not_found");
            // Not the right map, point on next value and loop back
            func.adda(0x2, reg_A0);
            func.addqw(0x4, reg_D0);
            func.bra("loop_start");
        }
    }
    func.label("return");
    func.movem_from_stack({ reg_D0, reg_D1, reg_D2 }, { reg_A0 });
    func.rts();

    return rom.inject_code(func);
}

static void add_gnome_text(Map* map, World& world, const Flag& gnome_flag, const std::string& text)
{
    gnome_text_map_ids_table.add_word(map->id());
    map->speaker_ids().emplace_back(GNOME_TEXT);
    uint16_t text_id = world.first_empty_game_string_id(0x4D);
    world.game_strings()[text_id] = text;
    gnome_command_words_table.add_word(0x1400 | (gnome_flag.byte << 3) | gnome_flag.bit);
    gnome_command_words_table.add_word(0xE000 | ((text_id - 0x4D) & 0x1FFF));
}


// =========================== WORLD EDITS ===========================

static MapPalette* build_blue_palace_palette(World& world)
{
    MapPalette* pal = new MapPalette(*world.map_palette(31));
    pal->at(0) = Color(0x20, 0x20, 0xC0);
    pal->at(1) = Color(0x60, 0x60, 0xC0);
    pal->at(2) = Color(0xA0, 0xA0, 0xC0);
    pal->at(4) = Color(0x40, 0x00, 0x40);
    pal->at(11) = Color(0x80, 0x80, 0x00);
    pal->at(12) = Color(0xC0, 0xE0, 0x00);
    world.add_map_palette(pal);
    return pal;
}

static MapPalette* build_blue_shrine_palette(World& world)
{
    MapPalette* pal = new MapPalette(*world.map(MAP_LAKE_SHRINE_SAVE_ROOM)->palette());
    pal->at(1) = Color(0x20, 0x20, 0x60);
    pal->at(2) = Color(0x20, 0x40, 0x80);
    pal->at(3) = Color(0x40, 0x60, 0xA0);
    pal->at(5) = Color(0x20, 0x20, 0x40);
    pal->at(8) = Color(0x20, 0x60, 0x80);
    world.add_map_palette(pal);
    return pal;
}

static void add_foxy_in_knt(RandomizerWorld& world)
{
    Map* map = world.map(MAP_PILLAR_HALLWAY_TO_NOLE);
    Entity* foxy = new Entity({
        .type_id = ENTITY_NPC_MAGIC_FOX,
        .position = Position(0x10, 0x2A, 0),
        .orientation = ENTITY_ORIENTATION_SW
    });
    map->add_entity(foxy);
    world.add_custom_dialogue_raw(foxy, "Foxy: What you are about to\naccomplish is grand, but can\nyou feel there is more to it?\x1e\n"
                                    "This island hides a bigger\nmystery, something that has\nyet to be solved...\x1e\n"
                                    "Perhaps you should take a\ncloser look at the\ncrypt of Mercator.\x03");
}

static Map* add_room_1(RandomizerWorld& world, MapPalette* pal)
{
    // Modify the first room of the Crypt as the entrance of the ARG dungeon
    Map* map = world.map(MAP_SECRET_1);
    map->clear_entities();
    map->layout(world.map(MAP_KN_PALACE_124)->layout());
    map->blockset(world.map(MAP_KN_PALACE_124)->blockset());
    map->palette(pal);
    map->background_music(10);
    world.map_connection(MAP_CRYPT_MAIN_HALL, map->id()).position_for_map(map->id(), 27, 23);

    // Add a Nole NPC explaining the riddle
    Entity* nole_npc = new Entity({
        .type_id = ENEMY_NOLE,
        .position = Position(0x13, 0x19, 0x2, true, true, false),
        .orientation = ENTITY_ORIENTATION_SE,
        .palette = 1
    });
    nole_npc->fightable(false);
    nole_npc->behavior_id(0);
    nole_npc->remove_when_flag_is_set(FLAG_ALL_VALID_EQUIPMENTS);
    map->add_entity(nole_npc);
    world.add_custom_dialogue_raw(nole_npc,
        "Nole: A LONG TIME AGO, A DRAGON\nNAMED GOLA CAST A SPELL ON ME.\x1e\n"
        "I KNOW YOU DEFEATED ME IN\nMANY OTHER WORLDS, AS MY CURSE\nIS TO SEE AND REMEMBER THEM ALL.\x1e\n"
        "ONLY YOU CAN END THIS, BUT\nTO GO ANY FURTHER, YOU WILL NEED\nTHE RIGHT TOOLS FOR THE JOB.\x03");

    // Add boulders that "explain" the equipments to put on
    Entity* first_boulder = new Entity({ .type_id = ENTITY_SMALL_GRAY_BOULDER, .position = Position(0x0F, 0x15, 0x7, true), .palette = 2 });
    map->add_entity(first_boulder);
    map->add_entity(new Entity({ .type_id = ENTITY_SMALL_RED_BOULDER,  .position = Position(0x10, 0x15, 0x7, true), .palette = 1, .entity_to_use_tiles_from = first_boulder }));
    map->add_entity(new Entity({ .type_id = ENTITY_SMALL_GRAY_BOULDER, .position = Position(0x13, 0x15, 0x7),       .palette = 2, .entity_to_use_tiles_from = first_boulder }));
    map->add_entity(new Entity({ .type_id = ENTITY_SMALL_GRAY_BOULDER, .position = Position(0x15, 0x15, 0x7, true), .palette = 2, .entity_to_use_tiles_from = first_boulder }));
    map->add_entity(new Entity({ .type_id = ENTITY_SMALL_GRAY_BOULDER, .position = Position(0x16, 0x15, 0x7, true), .palette = 2, .entity_to_use_tiles_from = first_boulder }));
    map->add_entity(new Entity({ .type_id = ENTITY_SMALL_RED_BOULDER,  .position = Position(0x19, 0x15, 0x7),       .palette = 1, .entity_to_use_tiles_from = first_boulder }));

    // Add a blocking platform that disappears when riddle is solved
    Entity* blocking_platform = new Entity({ .type_id = ENTITY_LARGE_DARK_PLATFORM, .position = Position(0x10, 0x1C, 0x1, true, true, false), .palette = 0 });
    blocking_platform->remove_when_flag_is_set(FLAG_ALL_VALID_EQUIPMENTS);
    map->add_entity(blocking_platform);

    return map;
}

static Map* add_room_2(RandomizerWorld& world, MapPalette* pal)
{
    Map* map = world.map(MAP_SECRET_2);
    map->visited_flag(Flag(0xC0, 0));
    map->layout(world.map(MAP_KN_PALACE_116)->layout());
    map->blockset(world.map(MAP_KN_PALACE_116)->blockset());
    map->palette(pal);
    map->background_music(10);

    Entity* spell_book = new Entity({ .type_id = 0xC0 + ITEM_SPELL_BOOK, .position = Position(0x10, 0x19, 0x1, true, true, false), .liftable = true });
    map->add_entity(spell_book);

    Entity* death_statue = new Entity({ .type_id = 0xC0 + ITEM_DEATH_STATUE, .position = Position(0x10, 0x13, 0x1, true, true, false), .liftable = true });
    map->add_entity(death_statue);

    return map;
}

static Map* add_room_3(RandomizerWorld& world, MapPalette* pal)
{
    Map* map = world.map(MAP_SECRET_3);
    map->visited_flag(Flag(0xC0, 0));
    map->layout(world.map(MAP_KN_PALACE_130)->layout());
    map->blockset(world.map(MAP_KN_PALACE_130)->blockset());
    map->palette(pal);
    map->background_music(10);

    Entity* blocking_platform = new Entity({
        .type_id = ENTITY_LARGE_DARK_PLATFORM,
        .position = Position(0x25, 0x1F, 0x1, true, true, false),
        .palette = 0
    });
    map->add_entity(blocking_platform);

    Entity* blocking_platform_2 = new Entity({
        .type_id = ENTITY_LARGE_DARK_PLATFORM,
        .position = Position(0x25, 0x17, 0x1, true, true, false),
        .palette = 0,
        .entity_to_use_tiles_from = blocking_platform
    });
    map->add_entity(blocking_platform_2);

    Entity* gnome_1 = new Entity({
        .type_id = ENTITY_NPC_GNOME,
        .position = Position(0x25, 0x1C, 0x2),
        .orientation = ENTITY_ORIENTATION_SE,
        .palette = 3
    });
    gnome_1->only_when_flag_is_set(FLAG_FOUND_GNOME_1);
    map->add_entity(gnome_1);

    Entity* gnome_2 = new Entity({
        .type_id = ENTITY_NPC_GNOME,
        .position = Position(0x29, 0x17, 0x2),
        .orientation = ENTITY_ORIENTATION_SW,
        .palette = 3,
        .entity_to_use_tiles_from = gnome_1
    });
    gnome_2->only_when_flag_is_set(FLAG_FOUND_GNOME_2);
    map->add_entity(gnome_2);

    Entity* gnome_3 = new Entity({
        .type_id = ENTITY_NPC_GNOME,
        .position = Position(0x26, 0x23, 0x2),
        .orientation = ENTITY_ORIENTATION_NE,
        .palette = 3
    });
    gnome_3->only_when_flag_is_set(FLAG_FOUND_GNOME_3);
    map->add_entity(gnome_3);

    Entity* gnome_4 = new Entity({
        .type_id = ENTITY_NPC_GNOME,
        .position = Position(0x28, 0x20, 0x2),
        .orientation = ENTITY_ORIENTATION_NW,
        .palette = 3,
        .entity_to_use_tiles_from = gnome_3
    });
    gnome_4->only_when_flag_is_set(FLAG_FOUND_GNOME_4);
    map->add_entity(gnome_4);

    // Add a Foxy NPC explaining the riddle
    Entity* foxy_npc = new Entity({
        .type_id = ENTITY_NPC_MAGIC_FOX,
        .position = Position(0x25, 0x1A, 0x2),
        .orientation = ENTITY_ORIENTATION_SE,
        .palette = 1
    });
    map->add_entity(foxy_npc);
    world.add_custom_dialogue_raw(foxy_npc, "Foxy: As you have heard\nfrom our mutual friend Nole,\x1e\n"
                                        "he needs someone strong enough\nto break Gola's curse\nonce and for all.\x1e\n"
                                        "Four gnomes happen to have left\nGreenmaze and are causing\na ruckus in Mercator.\x1e\n"
                                        "Bring them all back,\nand you shall pass...\x03");

    return map;
}

static void add_gnomes_in_mercator(World& world)
{
    // Underground traders shop
    Entity* gnome_1 = new Entity({
                                         .type_id = ENTITY_NPC_GNOME,
                                         .position = Position(0x21, 0x26, 0x1),
                                         .orientation = ENTITY_ORIENTATION_NW,
                                         .palette = 3,
                                         .talkable = true,
                                         .dialogue = 1
                                 });
    gnome_1->remove_when_flag_is_set(FLAG_FOUND_GNOME_1);
    world.map(0x294)->add_entity(gnome_1);
    add_gnome_text(world.map(0x294), world, FLAG_FOUND_GNOME_1, "Gnome: Ahah, you found me!\nI was thinking about opening\na shop in here...\x03");

    // Mercator dungeon depths
    Entity* gnome_2 = new Entity({
                                         .type_id = ENTITY_NPC_GNOME,
                                         .position = Position(0x23, 0x1B, 0x1, true, true),
                                         .orientation = ENTITY_ORIENTATION_SW,
                                         .palette = 3,
                                         .talkable = true,
                                         .dialogue = 3
                                 });
    gnome_2->remove_when_flag_is_set(FLAG_FOUND_GNOME_2);
    world.map(0x051)->add_entity(gnome_2);
    add_gnome_text(world.map(0x051), world, FLAG_FOUND_GNOME_2, "Gnome: Ahah, you found me!\nThese smashing and spinning\nboulders are so much fun!\x03");

    // Mercator castle kitchen
    Entity* gnome_3 = new Entity({
                                         .type_id = ENTITY_NPC_GNOME,
                                         .position = Position(0x17, 0x13, 0x0),
                                         .orientation = ENTITY_ORIENTATION_NW,
                                         .palette = 3,
                                         .talkable = true,
                                         .dialogue = 4
                                 });
    gnome_3->remove_when_flag_is_set(FLAG_FOUND_GNOME_3);
    world.map(0x045)->add_entity(gnome_3); // VARIANT of map 44!
    add_gnome_text(world.map(0x044), world, FLAG_FOUND_GNOME_3, "Gnome: Ahah, you found me!\nI came here because of the smell,\nand stayed for the taste.\x03");

    // Top of mercator castle walls
    Map* mercator_castle_court = world.map(0x020);
    // Fix improper palette use to free palette #3
    for(Entity* entity : mercator_castle_court->entities())
    {
        if(entity->palette() == 3)
            entity->palette(1);
    }
    // Add the gnome
    Entity* gnome_4 = new Entity({
                                         .type_id = ENTITY_NPC_GNOME,
                                         .position = Position(0x1F, 0x2E, 0xC),
                                         .orientation = ENTITY_ORIENTATION_NW,
                                         .palette = 3,
                                         .talkable = true,
                                         .dialogue = 1
                                 });
    gnome_4->remove_when_flag_is_set(FLAG_FOUND_GNOME_4);
    mercator_castle_court->add_entity(gnome_4);
    add_gnome_text(mercator_castle_court, world, FLAG_FOUND_GNOME_4, "Gnome: Ahah, you found me!\nThe view here sure is something.\nDid you find all my friends?\x03");

    gnome_text_map_ids_table.add_word(0xFFFF);
}

static Map* add_room_4(RandomizerWorld& world, MapPalette* pal)
{
    Map* map = world.map(MAP_SECRET_4);
    map->visited_flag(Flag(0xC0, 0));
    map->layout(world.map(MAP_KN_PALACE_138)->layout());
    map->blockset(world.map(MAP_KN_PALACE_138)->blockset());
    map->palette(pal);
    map->background_music(10);

    Entity* platform = new Entity({
        .type_id = ENTITY_LARGE_DARK_PLATFORM,
        .position = Position(0x14, 0x1D, 0x0, true, true, true),
        .palette = 0
    });
    map->add_entity(platform);

    Entity* npc_pockets = new Entity({
        .type_id = ENTITY_NPC_POCKETS,
        .position = Position(0x14, 0x17, 0x0, false, false, false),
        .orientation = ENTITY_ORIENTATION_NW,
        .palette = 1,
        .talkable = true
    });
    map->add_entity(npc_pockets);

    world.add_custom_dialogue_raw(npc_pockets, "Pockets: Hello Nigel! I heard\nthere was a secret temple being\nuncovered here, so I came.\x1e\n"
                                           "There are ancient writings on this\npillar, but I'm no archeologist...\x03");

    return map;
}

static Map* add_room_5(RandomizerWorld& world, MapPalette* pal)
{
    Map* map = world.map(MAP_SECRET_5);
    map->visited_flag(Flag(0xC0, 0));
    map->layout(world.map(MAP_LAKE_SHRINE_347)->layout());
    map->blockset(world.map(MAP_LAKE_SHRINE_347)->blockset());
    map->palette(pal);
    map->background_music(10);

    for(uint8_t z = 0x2 ; z <= 0x6 ; z += 0x2)
    {
        map->add_entity(new Entity({
            .type_id = ENTITY_ROCK,
            .position = Position(0x13, 0x19, z),
            .palette = 0
        }));
    }

    for(uint8_t x = 0x13; x <= 0x1F ; x += 0xC)
    {
        for(uint8_t y = 0x13; y <= 0x1F ; y += 0xC)
        {
            map->add_entity(new Entity({
                .type_id = 0xC0 + ITEM_GOLDEN_STATUE,
                .position = Position(x, y, 0x1F, true, true),
                .palette = 1,
                .liftable = true
            }));
        }
    }

    map->global_entity_mask_flags().emplace_back(GlobalEntityMaskFlag(FLAG_SOLVED_ROCKS_RIDDLE, 0));
    return map;
}

static Map* add_room_6(RandomizerWorld& world, MapPalette* pal)
{
    Map* map = world.map(MAP_SECRET_6);
    map->layout(world.map(MAP_LAKE_SHRINE_319)->layout());
    map->blockset(world.map(MAP_LAKE_SHRINE_319)->blockset());
    map->visited_flag(FLAG_SOLVED_ROCKS_RIDDLE);
    map->palette(pal);
    map->background_music(10);

    // Add a Nole NPC explaining the riddle
    Entity* nole_npc = new Entity({
        .type_id = ENEMY_NOLE,
        .position = Position(0x13, 0x19, 0x5, false),
        .orientation = ENTITY_ORIENTATION_SE,
        .palette = 1
    });
    nole_npc->fightable(false);
    nole_npc->behavior_id(0);
    map->add_entity(nole_npc);
    world.add_custom_dialogue_raw(nole_npc,
                              "Nole: AT LAST, YOU ARE READY\nFOR THE FINAL TRIAL. TO KNOW\nWHAT TO DO,\x1e\n"
                              "YOU WILL HAVE TO TALK TO HER.\nSHE HAS THE ANSWER ON HOW TO\nREACH THE HEART OF GOLA.\x1e\n"
                              "PREPARE WELL BEFORE GOING THERE,\nSINCE IT WILL BE HARDER\nTHAN ANYTHING YOU EVER FACED.\x03");

    return map;
}

static void add_statue_dialogues(RandomizerWorld& world)
{
    const std::string PREFIX_STRING = "Goddess: I shall give you one,\nand only one word...\x1e\nThis word is \"";
    const std::string SUFFIX_STRING = "\".\x03";
    const std::vector<std::string> WORDS = { "OPEN", "YOUR", "EYES", "TOWARDS", "MIRROR", "INSIDE", "MASSAN", "CAVE" };

    // Set all Goddess Statues in the game to give a hint
    std::vector<Entity*> statues;
    statues.emplace_back(world.map(MAP_DESTEL_WELL_290)->entity(0));        // Destel Well exit statue
    statues.emplace_back(world.map(MAP_486)->entity(0));                    // Mountainous Area bridge statue
    statues.emplace_back(world.map(MAP_576)->entity(2));                    // Center of Greenmaze statue
    statues.emplace_back(world.map(MAP_THIEVES_HIDEOUT_220)->entity(0));    // Thieves Hideout statue
    statues.emplace_back(world.map(MAP_KN_PALACE_117)->entity(3));          // King Nole's Palace statue
    statues.emplace_back(world.map(MAP_KN_CAVE_148)->entity(1));            // King Nole's Cave statue
    statues.emplace_back(world.map(MAP_KN_LABYRINTH_367)->entity(10));      // King Nole's Labyrinth statue
    statues.emplace_back(world.map(MAP_522)->entity(0));                    // Lake Shrine volcano statue

    for(size_t i=0 ; i<statues.size() ; ++i)
        world.add_custom_dialogue_raw(statues[i], PREFIX_STRING + WORDS[i] + SUFFIX_STRING);

    // Useless unreachable statue
    Map* map_lake_shrine_fountain = world.map(MAP_LAKE_SHRINE_342);
    Entity* invisible_cube = new Entity({
        .type_id = ENTITY_INVISIBLE_CUBE,
        .position = Position(0x19, 0x18, 0x3, true),
        .talkable = true,
        .dialogue = (uint8_t)map_lake_shrine_fountain->speaker_ids().size()
    });
    map_lake_shrine_fountain->add_entity(invisible_cube);
    world.add_custom_dialogue_raw(invisible_cube, "If only I could get closer to hear\nwhat the goddess has to say...\x03");
}

void PatchSecretARG::alter_world(World& w)
{
    RandomizerWorld& world = reinterpret_cast<RandomizerWorld&>(w);

    add_foxy_in_knt(world);

    MapPalette* blue_palace_palette = build_blue_palace_palette(world);
    MapPalette* blue_shrine_palette = build_blue_shrine_palette(world);

    _room_1 = add_room_1(world, blue_palace_palette);

    _room_2 = add_room_2(world, blue_palace_palette);
    _room_1->fall_destination(_room_2->id());

    _room_3 = add_room_3(world, blue_palace_palette);
    world.map_connections().emplace_back(MapConnection(_room_2->id(), 18, 15, _room_3->id(), 40, 37));

    add_gnomes_in_mercator(world);

    _room_4 = add_room_4(world, blue_palace_palette);
    world.map_connections().emplace_back(MapConnection(_room_3->id(), 37, 26, _room_4->id(), 26, 19));

    _room_5 = add_room_5(world, blue_shrine_palette);
    world.map_connections().emplace_back(MapConnection(_room_4->id(), 21, 21, _room_5->id(), 25, 34, 2));

    _room_6 = add_room_6(world, blue_shrine_palette);
    world.map_connections().emplace_back(MapConnection(_room_5->id(), 19, 25, _room_6->id(), 34, 25, 4));

    add_statue_dialogues(world);
}

// =========================== CODE INJECTIONS ===========================

static void add_teleport_to_golas_heart(md::ROM& rom, World& world)
{
    md::Code func_pre_use;
    func_pre_use.cmpiw(MAP_MASSAN_CAVE_806, addr_(0xFF1204));
    func_pre_use.bne("regular_test");
    {
        func_pre_use.cmpiw(0x2514, addr_(0xFF5400));
        func_pre_use.bne("regular_test");
        {
            func_pre_use.jmp(offsets::PROC_ITEM_USE_RETURN_SUCCESS_HAS_POST_USE);
        }
    }
    func_pre_use.label("regular_test");
    func_pre_use.jmp(world.item(ITEM_GOLA_EYE)->pre_use_address()); // Regular Gola's Eye test

    world.item(ITEM_GOLA_EYE)->pre_use_address(rom.inject_code(func_pre_use));

    // ---------------------------------------------------------------------------------------

    md::Code func_post_use;
    func_post_use.cmpiw(MAP_MASSAN_CAVE_806, addr_(0xFF1204));
    func_post_use.beq("tp_to_golas_heart");
    {
        func_post_use.jmp(world.item(ITEM_GOLA_EYE)->post_use_address()); // Regular Gola's Eye test
    }
    func_post_use.label("tp_to_golas_heart");
    func_post_use.movem_to_stack({ reg_D0_D7 }, { reg_A0_A6 });
    {
        func_post_use.trap(0, { 0x00, 0x07 }); // Play save game music
        func_post_use.jsr(0x29046); // Sleep_0 for 0x17 frames
        func_post_use.add_word(0x0000);
        func_post_use.jsr(0x852); // Restore BGM
        func_post_use.jsr(0x1592); // Save game to SRAM
        func_post_use.andib(0xF0, addr_(0xFF1052)); // Remove Spell Book
        func_post_use.addib(0x01, addr_(0xFF1052)); // ^^^
        func_post_use.andib(0x0F, addr_(0xFF1051)); // Remove Record Book
        func_post_use.addib(0x10, addr_(0xFF1051)); // ^^^
        func_post_use.movew(0x2F11, addr_(0xFF5400));
        func_post_use.movew(0x0708, addr_(0xFF5402)); // Reset subtiles position
        func_post_use.moveb(0x88, addr_(0xFF5404)); // Orientation
        func_post_use.trap(0, { 0x00, 0x4D });
        func_post_use.jsr(0x44C);
        func_post_use.movew(MAP_GOLAS_HEART_1, reg_D0); // Set MapID to Gola's Heart map
        func_post_use.movew(0x0000, addr_(0xFF5412)); // Reset player height
        func_post_use.moveb(0x00, addr_(0xFF5422)); // Reset ground height
        func_post_use.moveb(0x00, addr_(0xFF5439)); // ^
        func_post_use.jsr(0x1586E);
        func_post_use.jsr(0x434);
        func_post_use.clrb(reg_D0);
        func_post_use.jsr(0x2824);
        func_post_use.jsr(0x410);
        func_post_use.jsr(0x8EB4);
    }
    func_post_use.movem_from_stack({ reg_D0_D7 }, { reg_A0_A6 });
    func_post_use.rts();

    world.item(ITEM_GOLA_EYE)->post_use_address(rom.inject_code(func_post_use));
}

static void add_equipment_riddle_check(md::ROM& rom)
{
    // Function to set a flag if all the right equipments are equipped
    md::Code func;
    {
        func.cmpiw(MAP_CRYPT_656, addr_(0xFF1204));
        func.bne("no");
        func.btst(FLAG_ALL_VALID_EQUIPMENTS.bit, addr_(0xFF1000 + FLAG_ALL_VALID_EQUIPMENTS.byte));
        func.bne("no");
        func.cmpil(0x02000710, addr_(0xFF114E));
        func.bne("no");
        {
            // If good equipments are equipped in the right room, set a permanent flag and play a sound
            func.bset(FLAG_ALL_VALID_EQUIPMENTS.bit, addr_(0xFF1000 + FLAG_ALL_VALID_EQUIPMENTS.byte));
            func.movew(0xFFFF, addr_(0xFF5800));
            func.trap(0, { 0x00, 0x74 });
        }
        func.label("no");
        func.moveb(addr_(0xFF114F), reg_D0); // instruction replaced by the JSR
        func.rts();
    }
    uint32_t func_addr = rom.inject_code(func);
    rom.set_code(0x76EE, md::Code().jsr(func_addr));
}

static void add_gnome_riddle_check(md::ROM& rom, Map* map)
{
    md::Code func_init_map;
    {
        // Remove the Foxy if all gnomes have been found
        func_init_map.btst(FLAG_FOUND_GNOME_1.bit, addr_(0xFF1000 + FLAG_FOUND_GNOME_1.byte));
        func_init_map.beq("ret");
        func_init_map.btst(FLAG_FOUND_GNOME_2.bit, addr_(0xFF1000 + FLAG_FOUND_GNOME_2.byte));
        func_init_map.beq("ret");
        func_init_map.btst(FLAG_FOUND_GNOME_3.bit, addr_(0xFF1000 + FLAG_FOUND_GNOME_3.byte));
        func_init_map.beq("ret");
        func_init_map.btst(FLAG_FOUND_GNOME_4.bit, addr_(0xFF1000 + FLAG_FOUND_GNOME_4.byte));
        func_init_map.beq("ret");
        func_init_map.movew(0xFFFF, addr_(0xFF5780));
    }
    func_init_map.label("ret");
    func_init_map.rts();

    map->map_setup_addr(rom.inject_code(func_init_map));
}

void PatchSecretARG::inject_code(md::ROM& rom, World& world)
{
    add_equipment_riddle_check(rom);
    add_gnome_riddle_check(rom, _room_3);
    add_teleport_to_golas_heart(rom, world);

    // Replace Ludwig text script by the gnome text function
    uint32_t gnome_text_map_ids_addr = rom.inject_bytes(gnome_text_map_ids_table);
    uint32_t gnome_dialogue_commands_block = rom.inject_bytes(gnome_command_words_table);
    uint32_t func_gnome_text = inject_func_gnome_text(rom, gnome_text_map_ids_addr, gnome_dialogue_commands_block);
    rom.set_code(0x267D8, md::Code().jmp(func_gnome_text));
}
