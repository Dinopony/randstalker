#include "../tools/megadrive/rom.hpp"
#include "../tools/megadrive/code.hpp"
#include "../randomizer_options.hpp"
#include "../world.hpp"

#include "../assets/blue_jewel.bin.hxx"
#include "../assets/green_jewel.bin.hxx"
#include "../assets/yellow_jewel.bin.hxx"

#include "../model/item.hpp"
#include "../model/map.hpp"

#include "../constants/offsets.hpp"
#include "../constants/item_codes.hpp"

/**
 * The effect of Pawn Ticket fits very well one of a consumable item, but didn't
 * work this way in the original game. Instead, a story flag was set that prevented
 * to use the item again, but it was still in inventory.
 * This function changes this.
 */
void make_pawn_ticket_consumable(md::ROM& rom)
{
    md::Code proc_consume_pawn_ticket;
    proc_consume_pawn_ticket.jsr(0x8B98); // ConsumeItem
    proc_consume_pawn_ticket.nop(3);
    rom.set_code(0x88D2, proc_consume_pawn_ticket);
}

/**
 * This function makes the key not consumed on use, since it is now a unique item
 * that needs to be used to open several doors.
 */
void make_key_not_consumed_on_use(md::ROM& rom)
{
    rom.set_code(0x8B34, md::Code().nop());
}

/**
 * Add a "golds over time" effect to the statue of Jypta, granting golds
 * by walking (just like the Healing Boots grant life).
 */
void add_statue_of_jypta_effect(md::ROM& rom)
{
    constexpr uint16_t GOLDS_PER_CYCLE = 0x0001;

    // ============== Function to handle walk abilities (healing boots, jypta statue...) ==============
    md::Code func_handle_walk_abilities;

    // If Statue of Jypta is owned, gain gold over time
    func_handle_walk_abilities.btst(0x5, addr_(0xFF104E));
    func_handle_walk_abilities.beq(3);
    func_handle_walk_abilities.movew(GOLDS_PER_CYCLE, reg_D0);
    func_handle_walk_abilities.jsr(0x177DC);   // rom.stored_address("func_earn_gold");

    // If Healing boots are equipped, gain life over time
    func_handle_walk_abilities.cmpib(0x7, addr_(0xFF1150));
    func_handle_walk_abilities.bne(4);
    func_handle_walk_abilities.movew(0x100, reg_D0);
    func_handle_walk_abilities.lea(0xFF5400, reg_A5);
    func_handle_walk_abilities.jsr(0x1780E);   // rom.stored_address("func_heal_hp");

    func_handle_walk_abilities.rts();

    uint32_t func_addr = rom.inject_code(func_handle_walk_abilities);

    // ============== Hook the function inside game code ==============
    rom.set_code(0x16696, md::Code().nop(5));
    rom.set_code(0x166D0, md::Code().jsr(func_addr).nop(4));
}

/**
 * Make the effect of Statue of Gaia and Sword of Gaia way faster, because reasons.
 */
static void quicken_gaia_effect(md::ROM& rom)
{
    constexpr uint8_t SPEEDUP_FACTOR = 3;

    rom.set_word(0x1686C, rom.get_word(0x1686C) * SPEEDUP_FACTOR);
    rom.set_word(0x16878, rom.get_word(0x16878) * SPEEDUP_FACTOR);
    rom.set_word(0x16884, rom.get_word(0x16884) * SPEEDUP_FACTOR);
}

/**
 * Make the lantern a flexible item, by actually being able to impact a predefined
 * table of "dark maps" and turning screen to pitch black if it's not owned.
 */
static void alter_lantern_handling(md::ROM& rom)
{
    // ----------------------------------------
    // Function to darken the color palette currently used to draw the map
    // Input :  D0 = AND mask to apply to palette
    md::Code func_darken_palette;

    func_darken_palette.movem_to_stack({ reg_D1, reg_D2 }, { reg_A0 });
    func_darken_palette.lea(0xFF0080, reg_A0);
    func_darken_palette.movew(0x20, reg_D1);
    func_darken_palette.label("loop");
        func_darken_palette.movew(addr_(reg_A0), reg_D2);
        func_darken_palette.andw(reg_D0, reg_D2);
        func_darken_palette.movew(reg_D2, addr_(reg_A0));
        func_darken_palette.adda(0x2, reg_A0);
        func_darken_palette.dbra(reg_D1, "loop");
    func_darken_palette.movem_from_stack({ reg_D1, reg_D2 }, { reg_A0 });
    func_darken_palette.rts();

    uint32_t func_darken_palette_addr = rom.inject_code(func_darken_palette);

    // ----------------------------------------
    // Function to check if the current room is supposed to be dark, and process differently 
    // depending on whether or not we own the lantern
    md::Code func_check_lantern;

    func_check_lantern.lea(rom.stored_address("data_dark_rooms"), reg_A0);
    func_check_lantern.label("loop_start");
    func_check_lantern.movew(addr_(reg_A0), reg_D0);
    func_check_lantern.bmi(14);
        func_check_lantern.cmpw(addr_(0xFF1204), reg_D0);
        func_check_lantern.bne(10);
            // We are in a dark room
            func_check_lantern.movem_to_stack({ reg_D0 }, {});
            func_check_lantern.btst(0x1, addr_(0xFF104D));
            func_check_lantern.bne(3);
                // Dark room with no lantern ===> darken the palette
                func_check_lantern.movew(0x0000, reg_D0);
                func_check_lantern.bra(2);

                // Dark room with lantern ===> use lantern palette
                func_check_lantern.movew(0x0CCC, reg_D0);
            func_check_lantern.jsr(func_darken_palette_addr);
            func_check_lantern.movem_from_stack({ reg_D0 }, {});
            func_check_lantern.rts();
        func_check_lantern.addql(0x2, reg_A0);
        func_check_lantern.bra("loop_start");
    func_check_lantern.clrb(reg_D7);
    func_check_lantern.rts();

    rom.set_code(0x87BE, func_check_lantern);

    // ----------------------------------------
    // Function to change the palette used on map transition (already exist in OG, slightly modified)
    md::Code func_change_map_palette;

    func_change_map_palette.cmpb(addr_(0xFF112F), reg_D4);
    func_change_map_palette.beq(12);
        func_change_map_palette.moveb(reg_D4, addr_(0xFF112F));
        func_change_map_palette.movel(addr_(offsets::MAP_PALETTES_TABLE_POINTER), reg_A0);
        func_change_map_palette.mulu(bval_(0x1A), reg_D4);
        func_change_map_palette.adda(reg_D4, reg_A0);
        func_change_map_palette.lea(0xFF0084, reg_A1);
        func_change_map_palette.movew(0x000C, reg_D0);
        func_change_map_palette.jsr(0x96A);
        func_change_map_palette.clrw(addr_(0xFF0080));
        func_change_map_palette.movew(0x0CCC, addr_(0xFF0082));
        func_change_map_palette.clrw(addr_(0xFF009E));
        func_change_map_palette.jsr(0x87BE);
    func_change_map_palette.rts();

    rom.set_code(0x2D64, func_change_map_palette);
    
    // ----------------------------------------
    // Pseudo-function used to extend the global palette init function, used to call the lantern check
    // after initing all palettes (both map & entities)
    md::Code ext_init_palette;

    ext_init_palette.movew(0x0CCC, addr_(0xFF00A2));
    ext_init_palette.add_bytes(rom.data_chunk(0x19520, 0x19526)); // Add the jsr that was removed for injection
    ext_init_palette.jsr(0x87BE);
    ext_init_palette.rts();
    uint32_t addr = rom.inject_code(ext_init_palette);
    rom.set_code(0x19520, md::Code().jsr(addr));

    // ----------------------------------------
    // Replace the "dark room" palette from King Nole's Labyrinth by the lit room palette
    for (uint8_t i = 0; i < 0x1A; ++i)
        rom.set_byte(offsets::KNL_DARK_ROOM_PALETTE + i, rom.get_byte(offsets::KNL_LIT_ROOM_PALETTE + i));
}

/**
 * The randomizer has an option to handle more jewels than the good old Red and Purple ones.
 * It can handle up to 9 jewels, but we only can afford having 5 unique jewel items.
 * This means there are two modes:
 *      - Unique jewels mode (1-5 jewels): each jewel has its own ID, name and sprite
 *      - Kazalt jewels mode (6+ jewels): jewels are one generic item that can be stacked up to 9 times
 *
 * This function handles the "unique jewels mode" by replacing useless items (priest books), injecting
 * new sprites and taking care of everything for this to happen.
 */
static void handle_additional_jewels(md::ROM& rom, const RandomizerOptions& options)
{
    // If we are in "Kazalt jewel" mode, don't do anything
    if(options.jewel_count() > MAX_INDIVIDUAL_JEWELS)
        return;
    
    if(options.jewel_count() >= 3)
    {
        // Add a sprite for green jewel and make the item use it
        uint32_t green_jewel_sprite_addr = rom.inject_bytes(GREEN_JEWEL_SPRITE, GREEN_JEWEL_SPRITE_SIZE);
        rom.set_long(offsets::ITEM_SPRITES_TABLE + (ITEM_GREEN_JEWEL * 0x4), green_jewel_sprite_addr); // 0x121648
    }
    if(options.jewel_count() >= 4)
    {
        // Add a sprite for blue jewel and make the item use it
        uint32_t blue_jewel_sprite_addr = rom.inject_bytes(BLUE_JEWEL_SPRITE, BLUE_JEWEL_SPRITE_SIZE);
        rom.set_long(offsets::ITEM_SPRITES_TABLE + (ITEM_BLUE_JEWEL * 0x4), blue_jewel_sprite_addr);
    }
    if(options.jewel_count() >= 5)
    {
        // Add a sprite for green jewel and make the item use it
        uint32_t yellow_jewel_sprite_addr = rom.inject_bytes(YELLOW_JEWEL_SPRITE, YELLOW_JEWEL_SPRITE_SIZE);
        rom.set_long(offsets::ITEM_SPRITES_TABLE + (ITEM_YELLOW_JEWEL * 0x4), yellow_jewel_sprite_addr);
    }

    // Make the Awakening Book (the only one remaining in churches) heal all status conditions
    rom.set_code(0x24F6C, md::Code().nop(6));
    rom.set_code(0x24FB8, md::Code().moveb(0xFF, reg_D0));

    // Change the behavior of AntiCurse and Detox books (now Yellow and Blue jewels) in shops
    rom.set_byte(0x24C40, 0x40);
    rom.set_byte(0x24C58, 0x40);
}

static uint32_t make_record_book_save_on_use(md::ROM& rom, const RandomizerOptions& options)
{
    constexpr uint32_t MAP_ENTRANCE_POSITION_ADDRESS = 0xFF0018;

    // =========== Procedures to store map and position after last map transition ===========
    md::Code func_store_position;
    func_store_position.movew(addr_(0xFF1204), addr_(MAP_ENTRANCE_POSITION_ADDRESS));
    func_store_position.movew(addr_(0xFF5400), addr_(MAP_ENTRANCE_POSITION_ADDRESS+2));
    func_store_position.rts();
    uint32_t func_store_position_addr = rom.inject_code(func_store_position);
    
    // Inject this func_store_position on every map change by hooking it into the BellCheck function.
    md::Code bell_check_injector;
    bell_check_injector.jsr(func_store_position_addr); 
    bell_check_injector.jsr(0x22ED0); // Call j_GetItemQtyAndMaxQty because we replaced it to hook this function
    bell_check_injector.rts();
    uint32_t bell_check_injector_addr = rom.inject_code(bell_check_injector);
    rom.set_long(0x669E, bell_check_injector_addr);

    // Game load transition injection
    md::Code func_load_game_and_store_pos;
    func_load_game_and_store_pos.jsr(0x15C2); // "func_load_game"
    func_load_game_and_store_pos.jsr(func_store_position_addr);
    func_load_game_and_store_pos.rts();
    uint32_t func_load_game_and_store_pos_addr = rom.inject_code(func_load_game_and_store_pos);
    rom.set_code(0xEF46, md::Code().jsr(func_load_game_and_store_pos_addr));

    // -------- Function to save game using record book --------
    // On record book use, set stored position and map as current to fool the save_game function, then call it
    // to save the game with this map and position. Then, restore Nigel's position and map as if nothing happened.
    md::Code func_use_record_book;
    func_use_record_book.movem_to_stack({ reg_D0, reg_D1 }, {});

    if(options.consumable_record_book())
    {
        func_use_record_book.moveb(reg_D0, addr_(0xFF1152));
        func_use_record_book.jsr(0x8B98); // ConsumeItem
    }

    func_use_record_book.movew(addr_(0xFF1204), reg_D0);
    func_use_record_book.movew(addr_(0xFF5400), reg_D1);
    
    func_use_record_book.movew(addr_(MAP_ENTRANCE_POSITION_ADDRESS), addr_(0xFF1204));
    func_use_record_book.movew(addr_(MAP_ENTRANCE_POSITION_ADDRESS+2), addr_(0xFF5400));
    func_use_record_book.jsr(0x1592); // "func_save_game"

    func_use_record_book.movew(reg_D0, addr_(0xFF1204));
    func_use_record_book.movew(reg_D1, addr_(0xFF5400));
    func_use_record_book.movem_from_stack({ reg_D0, reg_D1 }, {});

    func_use_record_book.rts();
    return rom.inject_code(func_use_record_book, "func_use_record_book");
}

static void make_spell_book_warp_to_start(md::ROM& rom)
{
    uint16_t spawnX = rom.get_byte(0x0027FD);
    uint16_t spawnZ = rom.get_byte(0x002805);
    uint16_t spawnPosition = (spawnX << 8) + spawnZ;
    uint16_t spawnMapID = rom.get_word(0x0027F4);

    // ------------- New spell book teleport function -------------
    md::Code spellBookFunction;
    spellBookFunction.movem_to_stack({ reg_D0_D7 }, { reg_A0_A6 });
    spellBookFunction.movew(spawnPosition, addr_(0xFF5400));
    spellBookFunction.movew(0x0708, addr_(0xFF5402)); // Reset subtiles position
    spellBookFunction.trap(0, { 0x00, 0x4D });
    spellBookFunction.jsr(0x44C);
    spellBookFunction.movew(spawnMapID, reg_D0); // Set MapID to spawn map
    spellBookFunction.movew(0x0000, addr_(0xFF5412)); // Reset player height
    spellBookFunction.moveb(0x00, addr_(0xFF5422)); // Reset ground height
    spellBookFunction.moveb(0x00, addr_(0xFF5439)); // ^
    spellBookFunction.jsr(0x1586E);
    spellBookFunction.jsr(0x434);
    spellBookFunction.clrb(reg_D0);
    spellBookFunction.jsr(0x2824);
    spellBookFunction.jsr(0x410);
    spellBookFunction.jsr(0x8EB4);
    spellBookFunction.movem_from_stack({ reg_D0_D7 }, { reg_A0_A6 });
    spellBookFunction.rts();
    rom.inject_code(spellBookFunction, "func_spell_book");

    // ------------- Extended item post use table -------------
    uint32_t newPostUseTableAddr = rom.reserve_data_block(0x2A+6, "new_post_use_table");
    // PostUseGarlic
    rom.set_long(newPostUseTableAddr, 0x00008BE8); 
    rom.set_word(newPostUseTableAddr+4, 0x9BFF);
    newPostUseTableAddr += 6;
    // PostUseEinsteinWhistle
    rom.set_long(newPostUseTableAddr, 0x00008BF2);
    rom.set_word(newPostUseTableAddr+4, 0xA0FF);
    newPostUseTableAddr += 6;
    // PostUsePostUseGolasEye
    rom.set_long(newPostUseTableAddr, 0x00008C7C); 
    rom.set_word(newPostUseTableAddr+4, 0xABFF);
    newPostUseTableAddr += 6;
    // PostUseIdolStone
    rom.set_long(newPostUseTableAddr, 0x00008C8E); 
    rom.set_word(newPostUseTableAddr+4, 0xB1FF);
    newPostUseTableAddr += 6;
    // PostUseKey
    rom.set_long(newPostUseTableAddr, 0x00008CB6); 
    rom.set_word(newPostUseTableAddr+4, 0xB2FF);
    newPostUseTableAddr += 6;
    // PostUseShortcake
    rom.set_long(newPostUseTableAddr, 0x00008CF6); 
    rom.set_word(newPostUseTableAddr+4, 0xB6FF);
    newPostUseTableAddr += 6;
    // PostUseSpellbook
    rom.set_long(newPostUseTableAddr, rom.stored_address("func_spell_book")); 
    rom.set_word(newPostUseTableAddr+4, 0xA4FF);
    newPostUseTableAddr += 6;
    // End
    rom.set_long(newPostUseTableAddr, 0xFFFFFFFF); 
    rom.set_word(newPostUseTableAddr+4, 0xFFFF);

    // ------------- New function to read the new table -------------
    md::Code newPostUseTableLookupProc;
    newPostUseTableLookupProc.moveb(addr_(0x00FF1152), reg_D0);
    newPostUseTableLookupProc.lea(rom.stored_address("new_post_use_table"), reg_A0);
    newPostUseTableLookupProc.label("newPostUseTableLookupProc_loop");
    newPostUseTableLookupProc.moveb(addr_(reg_A0, 0x04), reg_D2);
    newPostUseTableLookupProc.cmpib(0xFF, reg_D2);
    newPostUseTableLookupProc.beq(7);
    newPostUseTableLookupProc.cmpb(reg_D0, reg_D2);
    newPostUseTableLookupProc.beq(3);
    newPostUseTableLookupProc.addql(0x06, reg_A0);
    newPostUseTableLookupProc.bra("newPostUseTableLookupProc_loop");
    newPostUseTableLookupProc.movel(addr_(reg_A0), reg_A0);
    newPostUseTableLookupProc.jmp(addr_(reg_A0));
    newPostUseTableLookupProc.rts();
    uint32_t newPostUseTableLookupProcAddr = rom.inject_code(newPostUseTableLookupProc);

    // ------------- JMP to override old lookup function by the new one -------------
    md::Code postUseTableInjectorCall;
    postUseTableInjectorCall.jmp(newPostUseTableLookupProcAddr);
    rom.set_code(0x8BC8, postUseTableInjectorCall);

    // Replace "consume Spell Book" by "trigger post use effect"
    rom.set_long(0x88C6, 0x600002EA); // bra loc_8BB2 >>> Mark used item as "has post use effect"
}

static void add_functions_to_items_on_use(md::ROM& rom, const RandomizerOptions& options)
{
    make_spell_book_warp_to_start(rom);
    uint32_t func_use_record_book_addr = make_record_book_save_on_use(rom, options);

    // ------------- Extended item handling function -------------

    md::Code funcExtendedItemHandling;

    funcExtendedItemHandling.cmpib(ITEM_RECORD_BOOK, reg_D0);
    funcExtendedItemHandling.bne(3);
        funcExtendedItemHandling.jsr(func_use_record_book_addr);
        funcExtendedItemHandling.rts();
    funcExtendedItemHandling.cmpib(ITEM_SPELL_BOOK, reg_D0);
    funcExtendedItemHandling.bne(3);
        funcExtendedItemHandling.jsr(0xDC1C); // "func_abracadabra"
        funcExtendedItemHandling.rts();
    funcExtendedItemHandling.cmpib(ITEM_LITHOGRAPH, reg_D0);
    funcExtendedItemHandling.bne(3);
        funcExtendedItemHandling.movew(0x21, reg_D0); // Read game string 0x21
        funcExtendedItemHandling.jsr(0x22E90);
    funcExtendedItemHandling.rts();
    
    uint32_t funcExtendedItemHandlingAddr = rom.inject_code(funcExtendedItemHandling);

    rom.set_code(0x00DBA8, md::Code().jsr(funcExtendedItemHandlingAddr).nop(4));

    // -------------------- Other modifications ---------------------

    // To remove the "Nothing happened..." text, the item must be put in a list which has a finite size.
    // We replace the Blue Ribbon (0x18) by the Record Book (0x23) to do so.
    rom.set_byte(0x008642, 0x23);
    // Same for Lithograph (0x27) remplacing Lantern (0x1A)
    rom.set_word(0x008647, 0x6627);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void patch_item_behavior(md::ROM& rom, const RandomizerOptions& options, const World& world)
{
    make_pawn_ticket_consumable(rom);
    make_key_not_consumed_on_use(rom);
    add_statue_of_jypta_effect(rom);
    quicken_gaia_effect(rom);
    alter_lantern_handling(rom);
    handle_additional_jewels(rom, options);
    add_functions_to_items_on_use(rom, options);
}