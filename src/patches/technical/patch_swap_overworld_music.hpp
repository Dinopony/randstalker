#pragma once

#include <landstalker_lib/patches/game_patch.hpp>

/**
 * This patch swaps the two overworld music (before and after taking the boat to Verla), because some settings tend
 * to make the conditions to have the second music pretty rarely met.
 */
class PatchSwapOverworldMusic : public GamePatch
{
public:
    void alter_rom(md::ROM& rom) override
    {
        // Transform the BEQ into a BNE to invert the test result deciding which overworld music to play
        rom.set_byte(0x2A20, 0x66);
    }
};
