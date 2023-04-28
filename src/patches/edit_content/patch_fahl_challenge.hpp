#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include "../../logic_model/randomizer_world.hpp"

class PatchFahlChallenge : public GamePatch
{
public:
    void alter_rom(md::ROM& rom) override
    {
        // Remove dialogues where Fahl propose the player to give them money during the challenge
        rom.set_code(0x12D52, md::Code().nop(24));
    }

    void inject_data(md::ROM& rom, World& world) override
    {
        RandomizerWorld& randomizer_world = reinterpret_cast<RandomizerWorld&>(world);

        if(randomizer_world.fahl_enemies().size() > 50)
            throw LandstalkerException("Cannot put more than 50 enemies for Fahl challenge");

        ByteArray fahl_enemies_bytes;
        for(EntityType* entity_type : randomizer_world.fahl_enemies())
            fahl_enemies_bytes.add_byte(entity_type->id());

        rom.set_bytes(offsets::FAHL_ENEMIES_TABLE, fahl_enemies_bytes);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        RandomizerWorld& randomizer_world = reinterpret_cast<RandomizerWorld&>(world);

        // Fix multiple state enemies causing glitched sprites for the following enemies
        md::Code func_fix_multi_state_enemies;
        {
            func_fix_multi_state_enemies.moveb(0x00, addr_(reg_A1, 0x38));
            func_fix_multi_state_enemies.jmp(0x1A4418); // Put back the existing instruction where injected
        }
        uint32_t addr = rom.inject_code(func_fix_multi_state_enemies);
        rom.set_code(0x12CC4, md::Code().jsr(addr));

        // Set the end of the challenge at the number of fahl enemies in the world list
        rom.set_byte(0x12D87, (uint8_t)randomizer_world.fahl_enemies().size());
    }
};
