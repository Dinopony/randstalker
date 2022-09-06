#pragma once

#include <landstalker_lib/patches/game_patch.hpp>

/**
 * Usually, Einstein Whistle can only be used in front of the sacred trees at the end of Greenmaze in order to cut them.
 * This patch extends the zone where the item can be used to make it usable from behind the trees, which allows for new
 * routing options.
 */
class PatchAllowWhistleUsageBehindTrees : public GamePatch
{
public:
    void alter_rom(md::ROM& rom) override
    {
        // Alter the bounds of the zone where we can use Einstein Whistle, to make it usable from behind the trees
        rom.set_byte(0x889F, 0x06); // X width
        rom.set_byte(0x88AD, 0x1A); // Y min
        rom.set_byte(0x88B1, 0x08); // Y width
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        // Inside the "post use" function for Einstein Whistle, disable the cutscene where Friday sees the item above the
        // trees since it would trigger AFTER trees would be cut in the case of using  the new backwards route
        md::Code func_remove_friday_cutscene;
        func_remove_friday_cutscene.bset(0, addr_(0xFF1026));       // Old instruction that was scrapped to call this injected proc
        func_remove_friday_cutscene.movew(0x0000, addr_(0xFF57AA)); // Remove Friday cutscene
        func_remove_friday_cutscene.rts();
        uint32_t func_addr = rom.inject_code(func_remove_friday_cutscene);

        rom.set_code(0x8C72, md::Code().nop().jsr(func_addr));
    }
};
