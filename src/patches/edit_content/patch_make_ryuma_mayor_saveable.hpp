#pragma once

#include <landstalker_lib/patches/game_patch.hpp>

class PatchMakeRyumaMayorSaveable : public GamePatch
{
public:
    void alter_rom(md::ROM& rom) override
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

        // Change the second mayor reward from "fixed 100 golds" to "item with ID located at 0x2837F"
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
};