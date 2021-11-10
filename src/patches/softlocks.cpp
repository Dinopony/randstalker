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


////////////////////////////////////////////////////////////////////////////////////////////////////////

void patch_softlocks(md::ROM& rom, const RandomizerOptions& options, const World& world)
{
    fix_crypt_soflocks(rom);
    alter_labyrinth_rafts(rom);
    remove_logs_room_exit_check(rom);
}
