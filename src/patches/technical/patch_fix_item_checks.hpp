#pragma once

#include "landstalker_lib/patches/game_patch.hpp"

/**
 * This patch fixes item checks that rely on story flags instead of checking if the item is actually owned.
 */
class PatchFixItemChecks : public GamePatch
{
public:
    void alter_rom(md::ROM& rom) override
    {
        fix_axe_magic_check(rom);
        fix_safety_pass_check(rom);
        fix_armlet_check(rom);
        fix_sunstone_check(rom);
        fix_dog_talking_check(rom);
        alter_casino_ticket_handling(rom);
    }

private:
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
     * Usually, when talking to dogs, the game checks if we went through various story events looking at story flags.
     * This patch makes the game look if Einstein Whistle is owned instead of looking at those flags.
     */
    static void fix_dog_talking_check(md::ROM& rom)
    {
        // Massan dog
        // 0x0253C0:
        // Before:	01 24 (0124 >> 3 = 24 and 0124 & 7 = 04 -----> bit 4 of FF1024)
        // After:	02 81 (0281 >> 3 = 50 and 0281 & 7 = 01 -----> bit 1 of FF1050)
        rom.set_word(0x0253C0, 0x0281);

        // Other dogs
        std::vector<uint32_t> dog_cutscenes = { 0x26238, 0x2647E, 0x26A40, 0x272B4 };
        for(uint32_t addr : dog_cutscenes)
        {
            uint32_t dialogue_without_whistle_offset = rom.get_word(addr + 6);
            uint32_t dialogue_with_whistle_offset = rom.get_word(addr + 10);

            md::Code cutscene;
            {
                uint32_t offset = 0x25D96 - (addr + 2); // 0x25D96 = CheckFlagAndDisplayMessage
                cutscene.add_word(0x6100);
                cutscene.add_word((uint16_t) offset);

                cutscene.add_word(0x0281); // 0281 (0281 >> 3 = 50 and 0281 & 7 = 01 -----> bit 1 of FF1050)

                cutscene.add_word(dialogue_with_whistle_offset);
                cutscene.add_word(dialogue_without_whistle_offset);

                cutscene.rts();
            }

            rom.set_code(addr, cutscene);
        }
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
};



