#pragma once

#include <landstalker-lib/patches/game_patch.hpp>

/**
 * This patch removes the core feature of the game where Friday revives Nigel using an EkeEke if he dies.
 * Disabling this makes the game significantly harder and more frustrating.
 */
class PatchRemoveEkeEkeAutoRevive : public GamePatch
{
public:
    void alter_rom(md::ROM& rom) override
    {
        // Change the BEQ into a BRA, making death always a game over
        rom.set_byte(0x10BFA, 0x60);
    }
};
