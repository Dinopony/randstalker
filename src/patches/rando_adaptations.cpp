#include <landstalker_lib/md_tools.hpp>
#include <landstalker_lib/model/world.hpp>
#include <landstalker_lib/model/item.hpp>
#include <landstalker_lib/model/map.hpp>
#include <landstalker_lib/model/entity.hpp>
#include <landstalker_lib/constants/item_codes.hpp>
#include <landstalker_lib/constants/map_codes.hpp>
#include <landstalker_lib/constants/entity_type_codes.hpp>
#include <landstalker_lib/constants/flags.hpp>
#include "../randomizer_options.hpp"

static void alter_lifestock_handling_in_shops(md::ROM& rom)
{
    // Make Lifestock prices the same over all shops
    for (uint32_t addr = 0x024D34; addr <= 0x024EAE; addr += 0xE)
        rom.set_byte(addr + 0x03, 0x10);

    // Remove the usage of "bought lifestock in shop X" flags 
    for (uint32_t addr = 0x009D18; addr <= 0x009D33; addr += 0xE)
        rom.set_byte(addr, 0xFF);
}

/**
 * Change Waterfall Shrine entrance check from "Talked to Prospero" to "What a noisy boy!", removing the need*
 * of talking to Prospero (which we couldn't do anyway because of the story flags).
 */
static void alter_waterfall_shrine_secret_stairs_check(md::ROM& rom)
{
    // 0x005014:
        // Before:	00 08 (bit 0 of FF1000)
        // After:	02 09 (bit 1 of FF1002)
    rom.set_word(0x005014, 0x0209);
}

/**
 * Change the flag checked for teleporter appearance from "saw the duke Kazalt cutscene" to "has visited four white golems room in King Nole's Cave"
 */
static void alter_king_nole_cave_teleporter_to_mercator_condition(md::ROM& rom, const World& world)
{
    world.map(MAP_KN_CAVE_ROOM_WITH_WHITE_GOLEMS_VARIANT)->visited_flag(FLAG_OPENED_MERCATOR_TELEPORTER_TO_KNC);
}

static void make_ryuma_mayor_saveable(md::ROM& rom)
{
    // Disable the cutscene (CSA_0068) when opening vanilla lithograph chest
    rom.set_code(0x136BE, md::Code().rts());

    // Shifts the boss cutscene right before to make room for one more instruction in mayor cutscene
    rom.set_long(0x28362, rom.get_long(0x28364));
    rom.set_word(0x2546A, rom.get_word(0x2546A) - 2);

    // Set the bit 1 of flag 1004 as the first instruction in mayor's cutscene, and move starting
    // offset of this cutscene accordingly
    rom.set_word(0x28366, 0x1421);
    rom.set_word(0x2546C, 0x2F39);

    // Edit Friday blocker behavior in the treasure room
    rom.set_word(0x9BA62, 0xFEFE);
}

static void fix_ryuma_mayor_reward(md::ROM& rom)
{
    // Change the second reward from "fixed 100 golds" to "item with ID located at 0x2837F"
    rom.set_byte(0x2837E, 0x00);
    rom.set_word(0x28380, 0x17E8);

    // Remove the "I think we need an exorcism" dialogue for the mayor when progression flags are much further in the game
    rom.set_word(0x2648E, 0xF908); // CheckFlagAndDisplayMessage
    rom.set_word(0x26490, 0x0023); // on bit 3 of flag FF1004
    rom.set_word(0x26492, 0x1801); // Script ID for post-reward dialogue
    rom.set_word(0x26494, 0x17FF); // Script ID for reward cutscene
    rom.set_code(0x26496, md::Code().rts());
    // Clear out the rest
    rom.set_long(0x26498, 0xFFFFFFFF);
}

/**
 * In the original game, you need to save Tibor to make teleport trees usable.
 * This removes this requirement.
 */
static void remove_tibor_requirement_to_use_trees(md::ROM& rom)
{
    // Remove the check of the "completed Tibor sidequest" flag to make trees usable
    rom.set_code(0x4E4A, md::Code().nop(5));
}

/**
 * Remove the "shop/church" flag on the priest room of Mir Tower to make its items on ground work everytime
 */
static void fix_mir_tower_priest_room_items(md::ROM& rom)
{
    // TODO: Handle ShopScript
    // 0x024E5A:
        // Before:	0307
        // After:	7F7F
    rom.set_word(0x024E5A, 0x7F7F);
}

/**
 * In the priest rom of King Nole's Labyrinth, there is a chest in the save room that has a unique behavior.
 * It is "infinite" in vanilla game and can be open as many times as we want, to get as many EkeEke as we need
 * to complete the game. This function removes this behavior by turning it into a regular chest.
 */
static void fix_knl_priest_room_infinite_chest(md::ROM& rom)
{
    rom.set_code(0x9EB9C, md::Code().nop(5));
}

static void prevent_hint_item_save_scumming(md::ROM& rom)
{
    md::Code func_save_on_buy;
    // Redo instructions that were removed by injection
    func_save_on_buy.movew(reg_D2, reg_D0);
    func_save_on_buy.jsr(0x291D6); 
    // Save game
    func_save_on_buy.jsr(0x1592);
    func_save_on_buy.rts();
    uint32_t func_save_on_buy_addr = rom.inject_code(func_save_on_buy);

    rom.set_code(0x24F3E, md::Code().jsr(func_save_on_buy_addr));
}

static void fix_crypt_softlocks(md::ROM& rom, World& world)
{
    // 1) Remove the check "if shadow mummy was beaten, raft mummy never appears again"
    // 0x019DF6:
        // Before:	0839 0006 00FF1014 (btst bit 6 in FF1014) ; 66 14 (bne $19E14)
        // After:	4EB9 00019E14 (jsr $19E14; 4E71 4E71 (nop nop)
    rom.set_code(0x19DF6, md::Code().nop(5));

    // 2) Change the shadow mummy appearance from "if armlet is owned" to "chest was opened"
    // 0x0117E8:
        // Before:	103C 001F ; 4EB9 00022ED0 ; 4A41 ; 6B00 F75C (bmi $10F52)
        // After:	0839 0002 00FF1097 (btst 2 FF1097)	; 6700 F75C (bne $10F52)
    md::Code inject_change_crypt_exit_check;
    inject_change_crypt_exit_check.btst(0x2, addr_(0xFF1097));
    inject_change_crypt_exit_check.nop(2);
    rom.set_code(0x117E8, inject_change_crypt_exit_check);
    rom.set_byte(0x117F4, 0x67); // Turn the BMI into a BEQ
}

/**
 * Change the rafts logic so we can take them several times in a row, preventing from getting softlocked by missing chests
 */
static void alter_labyrinth_rafts(md::ROM& rom)
{
    // The trick here is to use flag 1001 (which resets on every map change) to correctly end the cutscene 
    // while discarding the "raft already taken" state as early as the player moves to another map.
    rom.set_word(0x09E031, 0x0100);
    rom.set_word(0x09E034, 0x0100);
    rom.set_word(0x09E04E, 0x0100);
    rom.set_word(0x09E051, 0x0100);
}

/**
 * Set the story progress at least as "Duke fled to Kazalt" state, whatever the current progress is
 */
static void set_story_as_advanced(md::ROM& rom)
{
    rom.set_word(0x25324, 0x0000);
}

static void remove_verla_soldiers_on_verla_spawn(World& world)
{
    if(world.spawn_location().node_id() == "verla")
    {
        world.starting_flags().emplace_back(FLAG_VERLA_WHITE_KNIGHTS_KILLED);
        world.starting_flags().emplace_back(FLAG_VERLA_GOLDEN_KNIGHT_KILLED);
    }
}

static void untangle_verla_mines_flags(World& world)
{
    // Make Slasher appearance not dependant on Marley being killed
    world.map(MAP_VERLA_MINES_SLASHER_ARENA)->global_entity_mask_flags().clear();
}

static void put_dex_back_in_verla_mines(World& world)
{
    Map* map = world.map(MAP_VERLA_MINES_DEX_ARENA);

    // Clear global masks
    map->global_entity_mask_flags().clear();

    // Remove townsfolk and invisible cube
    map->remove_entity(4);
    map->remove_entity(3);
    map->remove_entity(2);
    map->remove_entity(1);

    // Add a new door
    map->add_entity(new Entity({
        .type_id = ENTITY_GATE_NORTH,
        .position = Position(20, 17, 3),
        .behavior_id = 525, // Open when Dex is killed
        .mask_flags = { EntityMaskFlag(false, FLAG_DEX_KILLED) },
    }));
}

void remove_music(md::ROM& rom)
{
    constexpr uint8_t MUSIC_SILENT = 0x20;

    for(uint32_t addr=0x2A32 ; addr < 0x2A44 ; ++addr)
        rom.set_byte(addr, MUSIC_SILENT);

    rom.set_byte(0x155EB, MUSIC_SILENT); // Last boss cutscene
    rom.set_byte(0x27721, MUSIC_SILENT); // Last boss cutscene
    rom.set_byte(0x15523, MUSIC_SILENT); // Last boss music
    rom.set_byte(0x9EBE3, MUSIC_SILENT); // Credits music

    // Boss music
    rom.set_byte(0x9D6A1, MUSIC_SILENT);
    rom.set_byte(0x9D747, MUSIC_SILENT);
    rom.set_byte(0x9E195, MUSIC_SILENT);
    rom.set_byte(0x9E2C1, MUSIC_SILENT);
    rom.set_byte(0x9E57C, MUSIC_SILENT);
}

void patch_rando_adaptations(md::ROM& rom, const RandomizerOptions& options, World& world)
{
    set_story_as_advanced(rom);

    alter_lifestock_handling_in_shops(rom);
    alter_waterfall_shrine_secret_stairs_check(rom);
    alter_king_nole_cave_teleporter_to_mercator_condition(rom, world);
    make_ryuma_mayor_saveable(rom);
    fix_ryuma_mayor_reward(rom);
    if (options.remove_tibor_requirement())
        remove_tibor_requirement_to_use_trees(rom);

    fix_mir_tower_priest_room_items(rom);
    fix_knl_priest_room_infinite_chest(rom);
    prevent_hint_item_save_scumming(rom);
    fix_crypt_softlocks(rom, world);
    alter_labyrinth_rafts(rom);
    remove_verla_soldiers_on_verla_spawn(world);
    untangle_verla_mines_flags(world);
    put_dex_back_in_verla_mines(world);
}

