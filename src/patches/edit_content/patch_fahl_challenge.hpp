#pragma once

#include <landstalker_lib/patches/game_patch.hpp>

class PatchFahlChallenge : public GamePatch
{
public:
    void alter_rom(md::ROM& rom) override
    {
        // Remove dialogues where Fahl propose the player to give them money during the challenge
        rom.set_code(0x12D52, md::Code().nop(24));
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        // Fix multiple state enemies causing glitched sprites for the following enemies
        md::Code func_fix_multi_state_enemies;
        {
            func_fix_multi_state_enemies.moveb(0x00, addr_(reg_A1, 0x38));
            func_fix_multi_state_enemies.jmp(0x1A4418); // Put back the existing instruction where injected
        }
        uint32_t addr = rom.inject_code(func_fix_multi_state_enemies);
        rom.set_code(0x12CC4, md::Code().jsr(addr));

        // Set the end of the challenge at the number of fahl enemies in the world list
        rom.set_byte(0x12D87, (uint8_t)world.fahl_enemies().size());
    }
};
