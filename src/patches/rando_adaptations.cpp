#include "../tools/megadrive/rom.hpp"
#include "../tools/megadrive/code.hpp"
#include "../randomizer_options.hpp"
#include "../world.hpp"
#include "../model/item.hpp"

///////////////////////////////////////////////////////////////////////////////////
//       RANDO ADAPTATIONS / ENHANCEMENTS
///////////////////////////////////////////////////////////////////////////////////

/**
 * In the original game, only 3 item IDs are reserved for gold rewards (3A, 3B, 3C)
 * Here, we moved the table of gold rewards to the end of the ROM so that we can handle 64 rewards up to 255 golds each.
 * In the new system, all item IDs after the "empty item" one (0x40 and above) are now gold rewards.
 */
void alter_gold_rewards_handling(md::ROM& rom)
{
    rom.set_byte(0x0070DF, ITEM_GOLDS_START); // cmpi 3A, D0 >>> cmpi 40, D0
    rom.set_byte(0x0070E5, ITEM_GOLDS_START); // subi 3A, D0 >>> subi 40, D0

    // ------------- Function to put gold reward value in D0 ----------------
    // Input: D0 = gold reward ID (offset from 0x40)
    // Output: D0 = gold reward value

    md::Code func_get_gold_reward;

    func_get_gold_reward.movem_to_stack({}, { reg_A0 });
    func_get_gold_reward.lea(rom.stored_address("data_gold_values"), reg_A0);
    func_get_gold_reward.moveb(addr_(reg_A0, reg_D0, md::Size::WORD), reg_D0);  // move.b (A0, D0.w), D0 : 1030 0000
    func_get_gold_reward.movem_from_stack({}, { reg_A0 });
    func_get_gold_reward.rts();

    uint32_t func_addr = rom.inject_code(func_get_gold_reward);

    // Set the call to the injected function
    // Before:      add D0,D0   ;   move.w (PC, D0, 42), D0
    // After:       jsr to injected function
    rom.set_code(0x0070E8, md::Code().jsr(func_addr));
}

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
    // --------------- Fixes for thieves hideout treasure room ---------------
    // Disable the cutscene when opening vanilla lithograph chest
    rom.set_code(0x136BE, md::Code().rts());

    // Disable Friday blocker in the treasure room
    rom.set_word(0x9BA62, 0xFEFE); // Why does that even work? I don't know...

    // Set the "Mayor freed" flag to byte 10D6, bit 4 (which happens to be the "Visited treasure room with mayor variant" flag)
    rom.set_word(0xA3F4, 0xD604);

    // Remove the "remove all NPCs on flag set" trigger in treasure room by putting it to an impossible flag
    rom.set_byte(0x1A9C0, 0x01);

    // Inject custom code "on map enter" to set the flag when it is convenient to do so
    md::Code func_on_map_enter;
    func_on_map_enter.lea(0xFF10C0, reg_A0); // Do the instruction that was replaced the hook call
    // When entering the cavern where we save Ryuma's mayor, set the flag "mayor is saved"
    func_on_map_enter.cmpib(0xE0, reg_D0);
    func_on_map_enter.bne(2);
        func_on_map_enter.bset(0x01, addr_(0xFF1004));
    func_on_map_enter.rts();

    uint32_t func_on_map_enter_addr = rom.inject_code(func_on_map_enter);
    rom.set_code(0x2952, md::Code().jsr(func_on_map_enter_addr));

    // --------------- Fixes for Ryuma's mayor reward ---------------
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

void add_jewel_check_for_kazalt_teleporter(md::ROM& rom, const RandomizerOptions& options)
{
    // ----------- Rejection textbox handling ------------
    md::Code func_reject_kazalt_tp;

    func_reject_kazalt_tp.jsr(0x22EE8); // open textbox
    func_reject_kazalt_tp.movew(0x22, reg_D0);
    func_reject_kazalt_tp.jsr(0x28FD8); // display text 
    func_reject_kazalt_tp.jsr(0x22EA0); // close textbox
    func_reject_kazalt_tp.rts();

    uint32_t func_reject_kazalt_tp_addr = rom.inject_code(func_reject_kazalt_tp);

    // ----------- Jewel checks handling ------------
    md::Code proc_handle_jewels_check;

    if(options.jewel_count() > MAX_INDIVIDUAL_JEWELS)
    {
        proc_handle_jewels_check.movem_to_stack({reg_D1},{});
        proc_handle_jewels_check.moveb(addr_(0xFF1054), reg_D1);
        proc_handle_jewels_check.andib(0x0F, reg_D1);
        proc_handle_jewels_check.cmpib(options.jewel_count(), reg_D1); // Test if red jewel is owned
        proc_handle_jewels_check.movem_from_stack({reg_D1},{});
        proc_handle_jewels_check.bgt(3);
            proc_handle_jewels_check.jsr(func_reject_kazalt_tp_addr);
            proc_handle_jewels_check.rts();
    }
    else
    {
        if(options.jewel_count() >= 1)
        {
            proc_handle_jewels_check.btst(0x1, addr_(0xFF1054)); // Test if red jewel is owned
            proc_handle_jewels_check.bne(3);
                proc_handle_jewels_check.jsr(func_reject_kazalt_tp_addr);
                proc_handle_jewels_check.rts();
        }
        if(options.jewel_count() >= 2)
        {
            proc_handle_jewels_check.btst(0x1, addr_(0xFF1055)); // Test if purple jewel is owned
            proc_handle_jewels_check.bne(3);
                proc_handle_jewels_check.jsr(func_reject_kazalt_tp_addr);
                proc_handle_jewels_check.rts();
        }
        if(options.jewel_count() >= 3)
        {
            proc_handle_jewels_check.btst(0x1, addr_(0xFF105A)); // Test if green jewel is owned
            proc_handle_jewels_check.bne(3);
                proc_handle_jewels_check.jsr(func_reject_kazalt_tp_addr);
                proc_handle_jewels_check.rts();
        }
        if(options.jewel_count() >= 4)
        {
            proc_handle_jewels_check.btst(0x5, addr_(0xFF1050)); // Test if blue jewel is owned
            proc_handle_jewels_check.bne(3);
                proc_handle_jewels_check.jsr(func_reject_kazalt_tp_addr);
                proc_handle_jewels_check.rts();
        }
        if(options.jewel_count() >= 5)
        {
            proc_handle_jewels_check.btst(0x1, addr_(0xFF1051)); // Test if yellow jewel is owned
            proc_handle_jewels_check.bne(3);
                proc_handle_jewels_check.jsr(func_reject_kazalt_tp_addr);
                proc_handle_jewels_check.rts();
        }
    }
    proc_handle_jewels_check.moveq(0x7, reg_D0);
    proc_handle_jewels_check.jsr(0xE110);  // "func_teleport_kazalt"
    proc_handle_jewels_check.jmp(0x62FA);

    uint32_t handle_jewels_addr = rom.inject_code(proc_handle_jewels_check);

    // This adds the jewels as a requirement for the Kazalt teleporter to work correctly
    rom.set_code(0x62F4, md::Code().jmp(handle_jewels_addr));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
//      RANDOMIZER RELATED BUGS
////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Add the ability to also trigger the volcano using the Sword of Gaia instead of only Statue of Gaia
 */
void make_sword_of_gaia_work_in_volcano(md::ROM& rom)
{
    md::Code proc_trigger_volcano;

    proc_trigger_volcano.cmpiw(0x20A, addr_(0xFF1204));
    proc_trigger_volcano.bne(4);
        proc_trigger_volcano.bset(0x2, addr_(0xFF1027));
        proc_trigger_volcano.jsr(0x16712);
        proc_trigger_volcano.rts();
    proc_trigger_volcano.jmp(0x16128);

    uint32_t proc_addr = rom.inject_code(proc_trigger_volcano);

    rom.set_code(0x1611E, md::Code().jmp(proc_addr));
}

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

/**
 * The original game has no way to make enemies unkillable, so it uses weird ultra high HP bosses
 * which don't represent their real life pool. This injection modifies the way this works to improve
 * this part of the engine by implementing a way to make them unkillable.
 * If specified in the options, this function also adds the check to prevent sacred trees from being
 * killed by feeding them money.
 */
void improve_checks_before_kill(md::ROM& rom, const RandomizerOptions& options)
{
    constexpr uint32_t ADDR_JMP_KILL = 0x16284;
    constexpr uint32_t ADDR_JMP_NO_KILL_YET = 0x16262;
    constexpr uint32_t ADDR_JMP_NO_KILL = 0x1627C;
    
    // Inject a new function which fixes the money value check on an enemy when it is killed, causing the tree glitch to be possible
    md::Code func_check_before_kill;
 
    // If enemy doesn't hold money, no kill yet
    func_check_before_kill.tstb(addr_(reg_A5, 0x36));
    if(options.fix_tree_cutting_glitch())
    {
        func_check_before_kill.beq(4);
            // Only allow the "killable because holding money" check if the enemy is not a tree
            func_check_before_kill.cmpiw(0x126, addr_(reg_A5, 0xA));
            func_check_before_kill.beq(2);
                func_check_before_kill.jmp(ADDR_JMP_KILL);
    }
    else
    {
        func_check_before_kill.beq(2);
            func_check_before_kill.jmp(ADDR_JMP_KILL);
    }
    // If enemy has "empty item" as a drop, make it unkillable
    func_check_before_kill.cmpib(0x3F, addr_(reg_A5, 0x77));
    func_check_before_kill.bne(3);
        func_check_before_kill.movew(0x0001, addr_(reg_A5, 0x3E));
        func_check_before_kill.rts();
    func_check_before_kill.jmp(ADDR_JMP_NO_KILL_YET);

    uint32_t func_addr = rom.inject_code(func_check_before_kill);

    // Call the injected function when killing an enemy
    rom.set_code(0x01625C, md::Code().jmp(func_addr));
}

/**
 * In original game, bosses have way higher HP than their real HP pool, and the game checks regularly
 * if their health goes below 0x100 to trigger a death cutscene. In an effort to normalize bosses HP
 * and allow us to make them have bigger HP pools, we lower all of those checks to verify that health
 * goes below 0x002
 */
void normalize_bosses_hp_checks(md::ROM& rom)
{
    // 1* Make the HP check below 0x0002 instead of below 0x6400
    // 2* Make the corresponding boss have 100 less HP
    // 3* Make the corresponding boss unkillable to ensure cutscene is played
    rom.set_byte(0x118DC, 0x0002);
    rom.set_byte(0x118EC, 0x0002);
    rom.set_word(0x11D38, 0x0002);
    rom.set_byte(0x11D80, 0x0002);
    rom.set_byte(0x11F8A, 0x0002);
    rom.set_byte(0x11FAA, 0x0002);
    rom.set_byte(0x12072, 0x0002);
    rom.set_byte(0x120C0, 0x0002);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void patch_rando_adaptations(md::ROM& rom, const RandomizerOptions& options, const World& world)
{
    // Rando adaptations / enhancements
    alter_gold_rewards_handling(rom);
    alter_lifestock_handling_in_shops(rom);
    alter_fahl_challenge(rom, world);
    alter_waterfall_shrine_secret_stairs_check(rom);
    alter_king_nole_cave_teleporter_to_mercator_condition(rom);
    make_ryuma_mayor_saveable(rom);
    if (options.remove_tibor_requirement())
        remove_tibor_requirement_to_use_trees(rom);
    if (options.use_armor_upgrades())
        handle_armor_upgrades(rom);

    // Logic enforcing
    add_jewel_check_for_kazalt_teleporter(rom, options);

    // Fix randomizer-related bugs
    make_sword_of_gaia_work_in_volcano(rom);
    fix_mir_tower_priest_room_items(rom);
    prevent_hint_item_save_scumming(rom);

    improve_checks_before_kill(rom, options);
    normalize_bosses_hp_checks(rom);
}

