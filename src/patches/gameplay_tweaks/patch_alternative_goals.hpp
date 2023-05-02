#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <utility>

class PatchAlternativeGoals : public GamePatch
{
private:
    std::string _goal;

public:
    explicit PatchAlternativeGoals(std::string  goal) : _goal(std::move(goal))
    {}

    void inject_code(md::ROM& rom, World& world) override
    {
        // If the goal is to reach Kazalt, make the Kazalt teleporter trigger the end cutscene
        if(_goal == "reach_kazalt")
            rom.set_code(0xE56A, md::Code().jmp(0x1556C));
    }
};


