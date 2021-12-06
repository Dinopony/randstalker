#include <md_tools.hpp>

#include "../randomizer_options.hpp"
#include "../world_model/world.hpp"
#include "../world_model/item.hpp"

#include "../constants/item_codes.hpp"

///////////////////////////////////////////////////////////////////////////////////
//       RANDO ADAPTATIONS / ENHANCEMENTS
///////////////////////////////////////////////////////////////////////////////////



void alter_lifestock_handling_in_shops(md::ROM& rom)
{
    // Make Lifestock prices the same over all shops
    for (uint32_t addr = 0x024D34; addr <= 0x024EAE; addr += 0xE)
        rom.set_byte(addr + 0x03, 0x10);

    // Remove the usage of "bought lifestock in shop X" flags 
    for (uint32_t addr = 0x009D18; addr <= 0x009D33; addr += 0xE)
        rom.set_byte(addr, 0xFF);
}

void alter_fahl_challenge(md::ROM& rom, const World& world)
{
    // Neutralize mid-challenge proposals for money
    rom.set_code(0x12D52, md::Code().nop(24));
    
    // Set the end of the challenge at the number of fahl enemies in the world list
    rom.set_byte(0x12D87, (uint8_t)world.fahl_enemies().size());
}

/**
 * Change Waterfall Shrine entrance check from "Talked to Prospero" to "What a noisy boy!", removing the need*
 * of talking to Prospero (which we couldn't do anyway because of the story flags).
 */
void alter_waterfall_shrine_secret_stairs_check(md::ROM& rom)
{
    // 0x005014:
        // Before:	00 08 (bit 0 of FF1000)
        // After:	02 09 (bit 1 of FF1002)
    rom.set_word(0x005014, 0x0209);
}

/**
 * Change the flag checked for teleporter appearance from "saw the duke Kazalt cutscene" to "has visited four white golems room in King Nole's Cave"
 */
void alter_king_nole_cave_teleporter_to_mercator_condition(md::ROM& rom)
{
    // 0x0050A0:
        // Before:	27 09 (bit 1 of flag 1027)
        // After:	D0 09 (bit 1 of flag 10D0)
    rom.set_word(0x0050A0, 0xD009);
    // 0x00509C:
        // Before:	27 11 (bit 1 of flag 1027)
        // After:	D0 11 (bit 1 of flag 10D0)
    rom.set_word(0x00509C, 0xD011);

    // We need to inject a procedure checking "is D0 equal to FF" to replace the "bmi" previously used which was preventing
    // from checking flags above 0x80 (the one we need to check is 0xD0).
    md::Code proc_extended_flag_check;

    proc_extended_flag_check.moveb(addr_(reg_A0, 0x2), reg_D0);     // 1028 0002
    proc_extended_flag_check.cmpib(0xFF, reg_D0);
    proc_extended_flag_check.bne(2);
    proc_extended_flag_check.jmp(0x4E2E);
    proc_extended_flag_check.jmp(0x4E20);

    uint32_t proc_addr = rom.inject_code(proc_extended_flag_check);

    // Replace the (move.b, bmi, ext.w) by a jmp to the injected procedure
    rom.set_code(0x004E18, md::Code().clrw(reg_D0).jmp(proc_addr));
}

void make_ryuma_mayor_saveable(md::ROM& rom)
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

void fix_ryuma_mayor_reward(md::ROM& rom)
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
void remove_tibor_requirement_to_use_trees(md::ROM& rom)
{
    // Remove the check of the "completed Tibor sidequest" flag to make trees usable
    rom.set_code(0x4E4A, md::Code().nop(5));
}

void handle_armor_upgrades(md::ROM& rom)
{
    // --------------- Alter item in D0 register function ---------------
    md::Code func_alter_item_in_d0;

    // Check if item ID is between 09 and 0C (armors). If not, branch to return.
    func_alter_item_in_d0.cmpib(ITEM_STEEL_BREAST, reg_D0);
    func_alter_item_in_d0.blt(13);
    func_alter_item_in_d0.cmpib(ITEM_HYPER_BREAST, reg_D0);
    func_alter_item_in_d0.bgt(11);

    // By default, put Hyper breast as given armor
    func_alter_item_in_d0.movew(ITEM_HYPER_BREAST, reg_D0);

    // If Shell breast is not owned, put Shell breast
    func_alter_item_in_d0.btst(0x05, addr_(0xFF1045));
    func_alter_item_in_d0.bne(2);
    func_alter_item_in_d0.movew(ITEM_SHELL_BREAST, reg_D0);

    // If Chrome breast is not owned, put Chrome breast
    func_alter_item_in_d0.btst(0x01, addr_(0xFF1045));
    func_alter_item_in_d0.bne(2);
    func_alter_item_in_d0.movew(ITEM_CHROME_BREAST, reg_D0);

    // If Steel breast is not owned, put Steel breast
    func_alter_item_in_d0.btst(0x05, addr_(0xFF1044));
    func_alter_item_in_d0.bne(2);
    func_alter_item_in_d0.movew(ITEM_STEEL_BREAST, reg_D0);

    func_alter_item_in_d0.rts();

    uint32_t func_alter_item_in_d0_addr = rom.inject_code(func_alter_item_in_d0);


    // --------------- Change item in reward box function ---------------
    md::Code func_change_item_reward_box;

    func_change_item_reward_box.jsr(func_alter_item_in_d0_addr);
    func_change_item_reward_box.movew(reg_D0, addr_(0xFF1196));
    func_change_item_reward_box.rts();

    uint32_t func_change_item_reward_box_addr = rom.inject_code(func_change_item_reward_box);

    // --------------- Change item given by taking item on ground function ---------------
    md::Code func_alter_item_given_by_ground_source;

    func_alter_item_given_by_ground_source.movem_to_stack({ reg_D7 }, { reg_A0 }); // movem D7,A0 -(A7)	(48E7 0180)

    func_alter_item_given_by_ground_source.cmpib(ITEM_HYPER_BREAST, reg_D0);
    func_alter_item_given_by_ground_source.bgt(9); // to movem
    func_alter_item_given_by_ground_source.cmpib(ITEM_STEEL_BREAST, reg_D0);
    func_alter_item_given_by_ground_source.blt(7);  // to movem

    func_alter_item_given_by_ground_source.jsr(func_alter_item_in_d0_addr);
    func_alter_item_given_by_ground_source.moveb(addr_(reg_A5, 0x3B), reg_D7);  // move ($3B,A5), D7	(1E2D 003B)
    func_alter_item_given_by_ground_source.subib(0xC9, reg_D7);
    func_alter_item_given_by_ground_source.cmpa(lval_(0xFF5400), reg_A5);
    func_alter_item_given_by_ground_source.blt(2);    // to movem
    func_alter_item_given_by_ground_source.bset(reg_D7, addr_(0xFF103F)); // set a flag when an armor is taken on the ground for it to disappear afterwards

    func_alter_item_given_by_ground_source.movem_from_stack({ reg_D7 }, { reg_A0 }); // movem (A7)+, D7,A0	(4CDF 0180)
    func_alter_item_given_by_ground_source.lea(0xFF1040, reg_A0);
    func_alter_item_given_by_ground_source.rts();

    uint32_t func_alter_item_given_by_ground_source_addr = rom.inject_code(func_alter_item_given_by_ground_source);

    // --------------- Change visible item for items on ground function ---------------
    md::Code func_alter_visible_item_for_ground_source;

    func_alter_visible_item_for_ground_source.movem_to_stack({ reg_D7 }, { reg_A0 });  // movem D7,A0 -(A7)

    func_alter_visible_item_for_ground_source.subib(0xC0, reg_D0);
    func_alter_visible_item_for_ground_source.cmpib(ITEM_HYPER_BREAST, reg_D0);
    func_alter_visible_item_for_ground_source.bgt(10); // to move D0 in item slot
    func_alter_visible_item_for_ground_source.cmpib(ITEM_STEEL_BREAST, reg_D0);
    func_alter_visible_item_for_ground_source.blt(8); // to move D0 in item slot
    func_alter_visible_item_for_ground_source.moveb(reg_D0, reg_D7);
    func_alter_visible_item_for_ground_source.subib(ITEM_STEEL_BREAST, reg_D7);
    func_alter_visible_item_for_ground_source.btst(reg_D7, addr_(0xFF103F));
    func_alter_visible_item_for_ground_source.bne(3);
    // Item was not already taken, alter the armor inside
    func_alter_visible_item_for_ground_source.jsr(func_change_item_reward_box_addr);
    func_alter_visible_item_for_ground_source.bra(2);
    // Item was already taken, remove it by filling it with an empty item
    func_alter_visible_item_for_ground_source.movew(ITEM_NONE, reg_D0);
    func_alter_visible_item_for_ground_source.moveb(reg_D0, addr_(reg_A1, 0x36)); // move D0, ($36,A1) (1340 0036)
    func_alter_visible_item_for_ground_source.movem_from_stack({ reg_D7 }, { reg_A0 }); // movem (A7)+, D7,A0	(4CDF 0180)
    func_alter_visible_item_for_ground_source.rts();

    uint32_t func_alter_visible_item_for_ground_source_addr = rom.inject_code(func_alter_visible_item_for_ground_source);

    // --------------- Hooks ---------------
    // In 'chest reward' function, replace the item ID move by the injected function
    rom.set_code(0x0070BE, md::Code().jsr(func_change_item_reward_box_addr));

    // In 'NPC reward' function, replace the item ID move by the injected function
    rom.set_code(0x028DD8, md::Code().jsr(func_change_item_reward_box_addr));

    // In 'item on ground reward' function, replace the item ID move by the injected function
    rom.set_word(0x024ADC, 0x3002); // put the move D2,D0 before the jsr because it helps us while changing nothing to the usual logic
    rom.set_code(0x024ADE, md::Code().jsr(func_change_item_reward_box_addr));

    // Replace 2928C lea (41F9 00FF1040) by a jsr to injected function
    rom.set_code(0x02928C, md::Code().jsr(func_alter_item_given_by_ground_source_addr));

    // Replace 1963C - 19644 (0400 00C0 ; 1340 0036) by a jsr to a replacement function
    rom.set_code(0x01963C, md::Code().jsr(func_alter_visible_item_for_ground_source_addr).nop());
}


///////////////////////////////////////////////////////////////////////////////////
//       LOGIC ENFORCING
///////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////////
//      RANDOMIZER RELATED BUGS
////////////////////////////////////////////////////////////////////////////////////////////////////////



/*
 * Remove the "shop/church" flag on the priest room of Mir Tower to make its items on ground work everytime
 */
void fix_mir_tower_priest_room_items(md::ROM& rom)
{
    // TODO: Handle ShopScript
    // 0x024E5A:
        // Before:	0307
        // After:	7F7F
    rom.set_word(0x024E5A, 0x7F7F);
}

void prevent_hint_item_save_scumming(md::ROM& rom)
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


////////////////////////////////////////////////////////////////////////////////////////////////////////
//      ORIGINAL GAME BUGS
////////////////////////////////////////////////////////////////////////////////////////////////////////



void fix_crypt_soflocks(md::ROM& rom)
{
    // 1) Remove the check "if shadow mummy was beaten, raft mummy never appears again"
    // 0x019DF6:
        // Before:	0839 0006 00FF1014 (btst bit 6 in FF1014) ; 66 14 (bne $19E14)
        // After:	4EB9 00019E14 (jsr $19E14; 4E71 4E71 (nop nop)
    rom.set_code(0x19DF6, md::Code().nop(5));

    // 2) Change the room exit check and shadow mummy appearance from "if armlet is owned" to "chest was opened"
    // 0x0117E8:
        // Before:	103C 001F ; 4EB9 00022ED0 ; 4A41 ; 6B00 F75C (bmi $10F52)
        // After:	0839 0002 00FF1097 (btst 2 FF1097)	; 6700 F75C (bne $10F52)
    md::Code inject_change_crypt_exit_check;
    inject_change_crypt_exit_check.btst(0x2, addr_(0xFF1097));
    inject_change_crypt_exit_check.nop(2);
    inject_change_crypt_exit_check.beq(); // beq $10F52
    rom.set_code(0x117E8, inject_change_crypt_exit_check);
}

/**
 * Change the rafts logic so we can take them several times in a row, preventing from getting softlocked by missing chests
 */
void alter_labyrinth_rafts(md::ROM& rom)
{
    // The trick here is to use flag 1001 (which resets on every map change) to correctly end the cutscene while discarding the "raft already taken" state 
    // as early as the player moves to another map.
    rom.set_word(0x09E031, 0x0100);
    rom.set_word(0x09E034, 0x0100);
    rom.set_word(0x09E04E, 0x0100);
    rom.set_word(0x09E051, 0x0100);
}


/**
 * In default game, the function that sets the flag to indicate that a room has been visited
 * is very limited in the number of addresses it can reach.
 * We modify it to be able to set any game flag, which is especially useful in some cases
 * >>> WHICH ONES?
 */
//void improve_visited_flag_setter(md::ROM& rom)
//{
//    rom.set_long(0x2954, 0xFF1000);
//}


////////////////////////////////////////////////////////////////////////////////////////////////////////

void patch_rando_adaptations(md::ROM& rom, const RandomizerOptions& options, const World& world)
{
    // Rando adaptations / enhancements
    alter_lifestock_handling_in_shops(rom);
    alter_fahl_challenge(rom, world);
    alter_waterfall_shrine_secret_stairs_check(rom);
    alter_king_nole_cave_teleporter_to_mercator_condition(rom);
//    improve_visited_flag_setter(rom);
    make_ryuma_mayor_saveable(rom);
    fix_ryuma_mayor_reward(rom);
    if (options.remove_tibor_requirement())
        remove_tibor_requirement_to_use_trees(rom);
    if (options.use_armor_upgrades())
        handle_armor_upgrades(rom);

    // Fix randomizer-related bugs
    fix_mir_tower_priest_room_items(rom);
    prevent_hint_item_save_scumming(rom);
    fix_crypt_soflocks(rom);
    alter_labyrinth_rafts(rom);
}

