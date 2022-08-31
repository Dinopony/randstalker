#pragma once

#include "landstalker_lib/patches/game_patch.hpp"

/**
 * In the base game, only statues of Gaia work to trigger the volcano leading to Lake Shrine.
 * instead of only Statue of Gaia
 * Add the ability to also trigger the volcano using the Sword of Gaia
 */
class PatchSwordOfGaiaInVolcano : public GamePatch
{
public:
    void inject_code(md::ROM& rom, World& world) override
    {
        md::Code proc_trigger_volcano;
        {
            proc_trigger_volcano.cmpiw(0x20A, addr_(0xFF1204));
            proc_trigger_volcano.bne("wrong_map");
            {
                proc_trigger_volcano.bset(0x2, addr_(0xFF1027));
                proc_trigger_volcano.jsr(0x16712);
                proc_trigger_volcano.rts();
            }
            proc_trigger_volcano.label("wrong_map");
            proc_trigger_volcano.jmp(0x16128);
        }
        uint32_t proc_addr = rom.inject_code(proc_trigger_volcano);

        rom.set_code(0x1611E, md::Code().jmp(proc_addr));
    }
};
