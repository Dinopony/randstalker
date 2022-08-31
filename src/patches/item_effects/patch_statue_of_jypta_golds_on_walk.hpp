#pragma once

#include "landstalker_lib/patches/game_patch.hpp"

/**
 * Add a "golds over time" effect to the statue of Jypta, granting golds by walking (similar to Healing Boots grant life).
 */
class PatchStatueOfJyptaGoldsOnWalk : public GamePatch
{
public:
    void inject_code(md::ROM& rom, World& world) override
    {
        constexpr uint16_t GOLDS_PER_CYCLE = 0x0001;

        md::Code func_handle_walk_abilities;
        {
            func_handle_walk_abilities.btst(0x5, addr_(0xFF104E));
            func_handle_walk_abilities.beq("healing_boots");
            {
                // If Statue of Jypta is owned, gain gold over time
                func_handle_walk_abilities.movew(GOLDS_PER_CYCLE, reg_D0);
                func_handle_walk_abilities.jsr(0x177DC);   // rom.stored_address("func_earn_gold");
            }
            func_handle_walk_abilities.label("healing_boots");
            func_handle_walk_abilities.cmpib(0x7, addr_(0xFF1150));
            func_handle_walk_abilities.bne("return");
            {
                // If Healing boots are equipped, gain life over time
                func_handle_walk_abilities.movew(0x100, reg_D0);
                func_handle_walk_abilities.lea(0xFF5400, reg_A5);
                func_handle_walk_abilities.jsr(0x1780E);   // rom.stored_address("func_heal_hp");
            }
            func_handle_walk_abilities.label("return");
            func_handle_walk_abilities.rts();
        }

        uint32_t func_addr = rom.inject_code(func_handle_walk_abilities);

        // ============== Hook the function inside game code ==============
        rom.set_code(0x16696, md::Code().nop(5));
        rom.set_code(0x166D0, md::Code().jsr(func_addr).nop(4));
    }
};
