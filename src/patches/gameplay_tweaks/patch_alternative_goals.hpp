#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <utility>
#include "../../constants/rando_constants.hpp"

class PatchAlternativeGoals : public GamePatch
{
private:
    std::string _goal;

public:
    explicit PatchAlternativeGoals(std::string  goal) : _goal(std::move(goal))
    {}

    void inject_code(md::ROM& rom, World& world) override
    {
        if(_goal == "reach_kazalt")
        {
            // If the goal is to reach Kazalt, make the Kazalt teleporter trigger the end cutscene
            rom.set_code(0xE56A, md::Code().jmp(0x1556C));
        }
        else if(_goal == "beat_dark_nole")
        {
            // If the goal is to beat Dark Nole, when taking the door to King Nole's treasure:
            // - save the game
            // - remove the Record Book and the Spell Book from inventory
            // - teleport the player to Gola's Heart
            uint32_t tp_func = inject_teleport_to_golas_heart(rom);
            rom.set_code(0x1528E, md::Code().jsr(tp_func).nop(9));
        }
    }

private:
    static uint32_t inject_teleport_to_golas_heart(md::ROM& rom)
    {
        md::Code func;
        {
            // Move Nigel
            func.movew(0x2F11, addr_(0xFF5400));    // Position
            func.movew(0x0708, addr_(0xFF5402));    // Subtiles
            func.moveb(0x88, addr_(0xFF5404));      // Orientation
            func.movew(0x0000, addr_(0xFF5412));    // Player Height
            func.moveb(0x00, addr_(0xFF5422));      // Reset ground height
            func.moveb(0x00, addr_(0xFF5439));      // ^^^

            // Set MapID to Gola's Heart first map
            func.movew(MAP_GOLAS_HEART_1, addr_(0xFF1204));
        }
        func.rts();

        return rom.inject_code(func);
    }
};


