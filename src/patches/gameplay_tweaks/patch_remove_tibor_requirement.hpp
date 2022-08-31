#pragma once

#include <landstalker_lib/patches/game_patch.hpp>

/**
 * In the original game, you need to save Tibor to make teleport trees usable.
 * This patch removes this requirement.
 */
class PatchRemoveTiborRequirement : public GamePatch
{
public:
    void alter_rom(md::ROM& rom) override
    {
        // Remove the check of the "completed Tibor sidequest" flag to make trees usable
        rom.set_code(0x4E4A, md::Code().nop(5));
    }
};
