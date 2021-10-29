#include "../tools/megadrive/rom.hpp"
#include "../tools/megadrive/code.hpp"
#include "../randomizer_options.hpp"
#include "../world.hpp"


/**
 * Usually, when trying to cut trees, the game checks if you have seen the
 * cutscene where Mir gives you the Axe Magic.
 * This function changes this to check if we own Axe Magic instead.
 */
static void fix_axe_magic_check(md::ROM& rom)
{
    // 0x16262:
        // Before:	[0839] btst 0 in FF1003
        // After:	[0839] btst 5 in FF104B
    rom.set_word(0x016264, 0x0005);
    rom.set_word(0x016268, 0x104B);
}

/**
 * Usually, when entering the "Mercator gates" map, the game puts a closed door
 * variant of the map as long as we didn't see the cutscene where the mayor gives
 * us the Safety Pass.
 * This function changes this to check if we own Safety Pass instead.
 */
static void fix_safety_pass_check(md::ROM& rom)
{
    // Change Mercator door opened check from bit 7 of flag 1004 to "Safety Pass owned"
    // 0x004FF8:
        // Before:	04 0F (0F & 7 = 7 -----> bit 7 of FF1004)
        // After:	59 0D (0D & 7 = 5 -----> bit 5 of FF1059)
    rom.set_word(0x004FF8, 0x590D);
}

/**
 * Usually, when trying to enter Mir Tower, the game checks if we have done the Crypt
 * instead of checking if we own the Armlet.
 * This functions fixes this.
 * 
 * That one comes in two blocks because an invisible wall was added in US version to
 * (badly) prevent armlet skipping, and it is a check of its own, so it also has to be
 * changed.
 * 
 * We also remove the consumption of the Armlet on use.
 */
static void fix_armlet_check(md::ROM& rom)
{
    // Actually changed the flag which is check for both triggers
    // 0x09C803:
        // Before:	14 06 (bit 6 of FF1014)
        // After:	4F 05 (bit 5 of FF104F)
    rom.set_word(0x09C803, 0x4F05);
    // 0x09C7F3: same as above
    rom.set_word(0x09C7F3, 0x4F05);
    
    // Prevent the game from removing the armlet from the inventory
    // 0x013A80: put a RTS instead of the armlet removal and all (not exactly sure why it works perfectly, but it does)
        // Before:  4E F9
        // After:	4E 75 (rts)
    rom.set_code(0x013A8A, md::Code().rts());
}

/**
 * Usually, when entering Ryuma's lighthouse top map, the game checks if we grabbed
 * Sun Stone in Greenmaze instead of checking if we actually own Sun Stone.
 * This function fixes this behavior.
 */
static void fix_sunstone_check(md::ROM& rom)
{
    // 0x09D091:
        // Before:	26 02 (bit 2 of FF1026)
        // After:	4F 01 (bit 1 of FF104F)
    rom.set_word(0x09D091, 0x4F01);
}

/**
 * Usually, when talking to Massan's doggo, the game checks if we went through the cutscene
 * where Greenmaze's lumberjack gives us Einstein Whistle, instead of checking if we actually
 * own the item.
 * This function fixes this behavior.
 */
static void fix_dog_talking_check(md::ROM& rom)
{
    // 0x0253C0:
        // Before:	01 24 (0124 >> 3 = 24 and 0124 & 7 = 04 -----> bit 4 of FF1024)
        // After:	02 81 (0281 >> 3 = 50 and 0281 & 7 = 01 -----> bit 1 of FF1050)
    rom.set_word(0x0253C0, 0x0281);
}

/**
 * Usually, when entering Mercator special shop, the game checks if we completed the 
 * dock traders sidequest. We chose to make the appearance of this shop bound to owning
 * Buyer's Card, since it's an item we usually get during that sidequest.
 */
static void alter_mercator_special_shop_check(md::ROM& rom)
{
    // 0x00A574:
        // Before:	2A 04 (bit 4 of FF102A)
        // After:	4C 05 (bit 5 of FF104C)
    rom.set_word(0x00A574, 0x4C05);
}

/**
 * This function changes the Casino Ticket handling to make it a unique item granting
 * lifetime access to the casino, instead of a consumable item that needs an infinite source
 * or otherwise could lock you out of the casino forever.
 */
static void alter_casino_ticket_handling(md::ROM& rom)
{
    // Remove ticket consumption on dialogue with the casino "bouncer"
    rom.set_code(0x277A8, md::Code().nop(2));

    // Remove Arthur giving his item another time if casino ticket is not owned
    rom.set_code(0x26BBA, md::Code().nop(5));
}

void make_massan_elder_reward_not_story_dependant(md::ROM& rom)
{
    // This item source writes its contents at this specific address, as specified
    // by the model files
    uint8_t reward_item_id = rom.get_byte(0x25F9E);

    md::Code elder_dialogue;
    elder_dialogue.btst(5, addr_(0xFF1002));
    elder_dialogue.bne(3);
        elder_dialogue.trap(0x01, { 0x00, 0x05 });
        elder_dialogue.rts();
        elder_dialogue.add_bytes({ 0xE0, 0xFD });
    elder_dialogue.btst(2, addr_(0xFF1004));
    elder_dialogue.bne(3);
        elder_dialogue.trap(0x01, { 0x00, 0x05 });
        elder_dialogue.rts();
        elder_dialogue.add_bytes({ 0x14, 0x22, 
                                   0x80, 0xFE, 
                                   0x80, 0xFF, 
                                   0x81, 0x00, 
                                   0x00, reward_item_id, 
                                   0x17, 0xE8, 
                                   0x18, 0x00, 
                                   0xE1, 0x01 });
    elder_dialogue.trap(0x01, { 0x00, 0x05 });
    elder_dialogue.rts();
    elder_dialogue.add_bytes({ 0xE1, 0x01 });

    uint32_t addr = rom.inject_code(elder_dialogue);

    rom.set_code(0x25F98, md::Code().jmp(addr).rts());

    rom.mark_empty_chunk(0x25FA0, 0x25FB1);
}

void make_gumi_boulder_push_not_story_dependant(md::ROM& rom)
{
    // Always remove Pockets from Gumi boulder map
    rom.set_word(0x1FED6, 0x7F7F);

    // Always show both bears in Gumi boulder map
    rom.set_word(0x1A630, 0x8002);
    rom.set_word(0x1A634, 0x8003);

    // Show the last boulder on ground only if upper boulder was pushed to prevent softlocks when coming from behind
    rom.set_word(0x1A62C, 0x82C6);
}

void make_lumberjack_reward_not_story_dependant(md::ROM& rom)
{
    rom.set_word(0x272C4, 0xEAFA); // Change from HandleProgressDependentDialogue to SetFlagBitOnTalking
    rom.set_word(0x272C6, 0x0124); // bit 4 of 1024
    rom.set_word(0x272C8, 0x0EF7); // Flag clear: Einstein whistle gift cutscene
    rom.set_word(0x272CA, 0x15D3); // Flag set: Thanks
    rom.set_word(0x272CC, 0x4E75); // rts

    // Shorten the cutscene by removing all references to the whistle (which we most likely won't get here)
    rom.set_word(0x28888, 0x0);
    rom.set_word(0x2888A, 0x0);
    rom.set_word(0x28890, 0x0);
    rom.set_word(0x28890, 0x0);
    rom.set_word(0x28892, 0x0);
    rom.set_word(0x28894, 0x0);

    // Remove Nigel turning around during the cutscene
    rom.set_code(0x14102, md::Code().nop(12));
}

/**
 * The "falling ribbon" item source in Mercator castle court is pretty dependant on story flags. In the original game,
 * the timeframe in the story where we can get it is tight. We get rid of any condition here, apart from checking
 * if item has already been obtained.
 */
void make_falling_ribbon_not_story_dependant(md::ROM& rom)
{
    // 0x00A466:
        // Before:  20 05 (bit 5 of flag 1020)
        // After:   3F 07 (bit 7 of flag 103F - never true)
    rom.set_word(0x00A466, 0x3F07);

    // 0x01A724:
        // Before:	16 22 (bit 1 of flag 1016)
        // After:	3F E2 (bit 7 of flag 103F - never true)
    rom.set_word(0x1A724, 0x3FE2);

    // Change falling item position from 1F to 20 to ensure it is taken by Nigel whatever its original position is
    rom.set_byte(0x09C59E, 0x20);

    // Remove the servant guarding the door, setting her position to 00 00
    rom.set_word(0x01BFCA, 0x0000);
}

void make_mercator_docks_shop_always_open(md::ROM& rom)
{
    // 0x01AA26:
        // Before:	0284 2A A2 (in map 284, check bit 5 of flag 102A)
        // After:	0000 5F E2 (in map 0, check bit 7 of flag 105F - never true)
    rom.set_word(0x01AA26, 0x0000);
    rom.set_word(0x01AA28, 0x5FE2);
}

void make_ryuma_shop_always_open(md::ROM& rom)
{
    // Make Ryuma's shop open without saving the mayor
    rom.set_byte(0x1A64D, 0x00);
}

void make_tibor_always_open(md::ROM& rom)
{
    // Make Tibor open without saving the mayor
    rom.set_byte(0x501D, 0x10);
}

void make_arthur_always_in_throne_room(md::ROM& rom)
{
    // Change the Arthur check giving casino tickets for him to be always here, instead of only after Lake Shrine
    // 0x01A904: 
        // Before:  AAA0 (bit 5 of flag 2A, affecting entity 0) 
        // After:   8000 (bit 0 of flag 00, affecting entity 0)
    rom.set_word(0x01A904, 0x8000);

    // 0x01A908:
        // Before:	AAA1 (bit 5 of flag 2A, affecting entity 1) 
        // After:	8001 (bit 0 of flag 00, affecting entity 1)
    rom.set_word(0x01A908, 0x8001);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////

void patch_story_dependencies(md::ROM& rom, const RandomizerOptions& options, const World& world)
{
    // Flag-reading changes (from story flag reading to inventory reading)
    fix_axe_magic_check(rom);
    fix_safety_pass_check(rom);
    fix_armlet_check(rom);
    fix_sunstone_check(rom);
    fix_dog_talking_check(rom);
    alter_mercator_special_shop_check(rom);
    alter_casino_ticket_handling(rom);

    make_massan_elder_reward_not_story_dependant(rom);
    make_gumi_boulder_push_not_story_dependant(rom);
    make_lumberjack_reward_not_story_dependant(rom);
    make_falling_ribbon_not_story_dependant(rom);
    make_mercator_docks_shop_always_open(rom);
    make_ryuma_shop_always_open(rom);
    make_tibor_always_open(rom);
    make_arthur_always_in_throne_room(rom);
}
