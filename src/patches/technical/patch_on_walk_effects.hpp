#pragma once

#include <landstalker-lib/patches/game_patch.hpp>

/**
 * Add handling for several procedures that trigger while the playing can walk
 */
class PatchOnWalkEffects : public GamePatch
{
private:
    bool _archipelago;

public:
    explicit PatchOnWalkEffects(bool archipelago) : _archipelago(archipelago)
    {}

    void inject_code(md::ROM& rom, World& world) override
    {
        uint32_t func_addr = inject_func_handle_item_abilities_on_walk(rom);
        rom.set_code(0x166D0, md::Code().jsr(func_addr).nop(4));

        if(_archipelago)
        {
            uint32_t func_handle_archipelago_checks = inject_func_handle_archipelago_checks(rom);
            rom.set_code(0x16696, md::Code().jsr(func_handle_archipelago_checks).nop(2));
        }
        else
        {
            rom.set_code(0x16696, md::Code().nop(5));
        }
    }

    uint32_t inject_func_handle_item_abilities_on_walk(md::ROM& rom)
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

        return rom.inject_code(func_handle_walk_abilities);
    }

private:
    uint32_t inject_func_handle_archipelago_checks(md::ROM& rom)
    {
        md::Code func;
        {
            // If an received item needs to be obtained, give the item and display a textbox
            func.cmpib(0xFF, addr_(0xFF0020));
            func.beq("deathlink");
            {
                func.clrw(addr_(0xFF1196));
                func.moveb(addr_(0xFF0020), addr_(0xFF1197));
                func.jsr(0x28EBA); // Receive item
                func.jsr(0x28FB8); // Close textbox
                func.addiw(1, addr_(0xFF107E));
                func.moveb(0xFF, addr_(0xFF0020));
            }
            // If a death was received while playing with deathlink, kill Nigel
            func.label("deathlink");
            func.cmpib(0x01, addr_(0xFF0021));
            func.bne("return");
            {
                func.movew(0x0000, addr_(0xFF543E));
                func.moveb(0x02, addr_(0xFF0021));
                func.movem_to_stack({ reg_D0 }, { reg_A0 });
                func.jsr(0x164EA);
                func.movem_from_stack({ reg_D0 }, { reg_A0 });
            }
            func.label("return");
        }
        func.rts();

        return rom.inject_code(func);
    }
};
