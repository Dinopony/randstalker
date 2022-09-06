#pragma once

#include "landstalker_lib/patches/game_patch.hpp"

/**
 * Make the effect of Statue of Gaia and Sword of Gaia way faster than in vanilla.
 */
class PatchFasterGaiaEffect : public GamePatch
{
public:
    void alter_rom(md::ROM& rom) override
    {
        constexpr uint8_t SPEEDUP_FACTOR = 3;

        rom.set_word(0x1686C, rom.get_word(0x1686C) * SPEEDUP_FACTOR);
        rom.set_word(0x16878, rom.get_word(0x16878) * SPEEDUP_FACTOR);
        rom.set_word(0x16884, rom.get_word(0x16884) * SPEEDUP_FACTOR);
    }
};
