#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <landstalker-lib/constants/map_codes.hpp>

/**
 * In vanilla game, after saving in a church, player was asked whether they want to continue the game or not.
 * This is not useful nowadays given the way the games are played, and makes the saving process overall very tedious.
 * This patch removes that prompt.
 */
class PatchRemoveContinuePromptAfterSaving : public GamePatch
{
public:
    void alter_rom(md::ROM& rom) override
    {
        rom.set_word(0x25022, 0x601A); // BRA over the whole prompt process

        rom.set_byte(0x27842, 0xE0); // Close textbox after regular priest announces save is completed
        rom.set_byte(0x27844, 0xE0); // Close textbox after regular priest announces save was not performed

        rom.set_byte(0x27940, 0xE0); // Close textbox after skeleton priest announces save is completed
        rom.set_byte(0x27942, 0xE0); // Close textbox after skeleton priest announces save was not performed

        rom.set_byte(0x25005, 0x06); // Reduce the waiting time for the save game music to start from 39 frames to 6 frames
    }
};
