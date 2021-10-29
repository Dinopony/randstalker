#include "../tools/megadrive/rom.hpp"
#include "../tools/megadrive/code.hpp"
#include "../randomizer_options.hpp"
#include "../world.hpp"

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
 * Usually, when trying to leave the room where you get Logs in the vanilla game without having taken both logs, a dwarf
 * comes and prevents you from leaving. Here, we remove that check since we cannot softlock anymore on the raft.
 */
void remove_logs_room_exit_check(md::ROM& rom)
{
    rom.set_code(0x011EC4, md::Code().bra());
}

/**
 * In the original game, coming back to Mir room after Lake Shrine would softlock you because Mir
 * would not be there. This check is removed to prevent any softlock and allow fighting Mir after having
 * done Lake Shrine.
 */
void fix_mir_after_lake_shrine_softlock(md::ROM& rom)
{
    // 0x01AA22:
        // Before:	0310 2A A2 (in map 310, check bit 5 of flag 102A)
        // After:	0000 5F E2 (in map 0, check bit 7 of flag 105F - never true)
    rom.set_word(0x01AA22, 0x0000);
    rom.set_word(0x01AA24, 0x5FE2);
}

void fix_reverse_greenmaze_fountain_softlock(md::ROM& rom)
{
    // Pressing the button behind the locked door now triggers the flag "Visited Greenmaze Crossroads map"
    rom.set_byte(0x9C66C, 0xF1);
    rom.set_byte(0x9C66D, 0x00);

    // Change the flag checked for lowering fountain to "Visited Greenmaze Crossroads map"
    rom.set_byte(0x500C, 0xF1);
    rom.set_byte(0x500D, 0x08);
}

/**
 * Make it so Lifestock chest near Fara in Swamp Shrine appears again when going back into the room afterwards, preventing any softlock there.
 */
void fix_fara_arena_chest_softlock(md::ROM& rom)
{
    // --------- Function to remove all entities but the chest when coming back in the room ---------
    md::Code func_remove_all_entities_but_chest_in_fara_room;

    func_remove_all_entities_but_chest_in_fara_room.movem_to_stack({ reg_D0_D7 }, { reg_A0_A6 });
    func_remove_all_entities_but_chest_in_fara_room.lea(0xFF5480, reg_A0);
    func_remove_all_entities_but_chest_in_fara_room.moveq(0xD, reg_D0);
    func_remove_all_entities_but_chest_in_fara_room.label("loop_remove_entities");
        func_remove_all_entities_but_chest_in_fara_room.movew(0x7F7F, addr_(reg_A0));
        func_remove_all_entities_but_chest_in_fara_room.adda(0x80, reg_A0);
    func_remove_all_entities_but_chest_in_fara_room.dbra(reg_D0, "loop_remove_entities");
    func_remove_all_entities_but_chest_in_fara_room.movem_from_stack({ reg_D0_D7 }, { reg_A0_A6 });
    func_remove_all_entities_but_chest_in_fara_room.rts();

    uint32_t addr = rom.inject_code(func_remove_all_entities_but_chest_in_fara_room);

    // Call the injected function
    rom.set_code(0x019BE0, md::Code().jsr(addr).nop());

    // --------- Moving the chest to the ground ---------
    rom.set_word(0x01BF6C, 0x1A93);
    rom.set_word(0x01BF6E, 0x0000);
    rom.set_word(0x01BF70, 0x0012);
    rom.set_word(0x01BF72, 0x0400);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void patch_softlocks(md::ROM& rom, const RandomizerOptions& options, const World& world)
{
    fix_crypt_soflocks(rom);
    alter_labyrinth_rafts(rom);
    remove_logs_room_exit_check(rom);
    fix_mir_after_lake_shrine_softlock(rom);
    fix_reverse_greenmaze_fountain_softlock(rom);
    fix_fara_arena_chest_softlock(rom);
}