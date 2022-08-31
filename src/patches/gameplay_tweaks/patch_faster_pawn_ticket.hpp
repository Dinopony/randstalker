#pragma once

#include "landstalker_lib/patches/game_patch.hpp"

/**
 * Pawn Ticket usage is very slow in vanilla game. This patch speeds it up to make it bearable.
 */
class PatchFasterPawnTicket : public GamePatch
{
public:
    void alter_rom(md::ROM& rom) override
    {
        constexpr uint8_t SPEEDUP_FACTOR = 3;

        rom.set_word(0x8920, rom.get_word(0x8920) / SPEEDUP_FACTOR);
    }
};
