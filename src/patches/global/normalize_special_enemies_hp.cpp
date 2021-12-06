#include <md_tools.hpp>

/**
 * The original game has no way to make enemies unkillable, so it uses weird ultra high HP bosses
 * which don't represent their real life pool. This injection modifies the way this works to improve
 * this part of the engine by implementing a way to make them unkillable.
 * If specified in the options, this function also adds the check to prevent sacred trees from being
 * killed by feeding them money.
 */
static void improve_checks_before_kill(md::ROM& rom, bool fix_tree_cutting_glitch)
{
    constexpr uint32_t ADDR_JMP_KILL = 0x16284;
    constexpr uint32_t ADDR_JMP_NO_KILL_YET = 0x16262;
    constexpr uint32_t ADDR_JMP_NO_KILL = 0x1627C;

    // Inject a new function which fixes the money value check on an enemy when it is killed, causing the tree glitch to be possible
    md::Code func_check_before_kill;

    // If enemy doesn't hold money, no kill yet
    func_check_before_kill.tstb(addr_(reg_A5, 0x36));
    if(fix_tree_cutting_glitch)
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


static void alter_bosses_hp_checks(md::ROM& rom)
{
    constexpr uint16_t NEW_THRESHOLD = 0x0002;

    // 1* Make the HP check below 0x0002 instead of below 0x6400
    // 2* Make the corresponding boss have 100 less HP
    // 3* Make the corresponding boss unkillable to ensure cutscene is played
    rom.set_byte(0x118DC, NEW_THRESHOLD);
    rom.set_byte(0x118EC, NEW_THRESHOLD);
    rom.set_word(0x11D38, NEW_THRESHOLD);
    rom.set_byte(0x11D80, NEW_THRESHOLD);
    rom.set_byte(0x11F8A, NEW_THRESHOLD);
    rom.set_byte(0x11FAA, NEW_THRESHOLD);
    rom.set_byte(0x12072, NEW_THRESHOLD);
    rom.set_byte(0x120C0, NEW_THRESHOLD);
}

/**
 * In original game, bosses have way higher HP than their real HP pool, and the game checks regularly
 * if their health goes below 0x100 to trigger a death cutscene. In an effort to normalize bosses HP
 * and allow us to make them have bigger HP pools, we lower all of those checks to verify that health
 * goes below 0x002 while putting a special procedure to ensure they are unkillable by the player
 */
void normalize_special_enemies_hp(md::ROM& rom, bool fix_tree_cutting_glitch)
{
    improve_checks_before_kill(rom, fix_tree_cutting_glitch);
    alter_bosses_hp_checks(rom);
}