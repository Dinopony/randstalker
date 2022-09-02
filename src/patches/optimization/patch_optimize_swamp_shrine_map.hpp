#pragma once

#include "landstalker_lib/patches/game_patch.hpp"
#include "landstalker_lib/model/map.hpp"
#include "landstalker_lib/model/entity.hpp"
#include "landstalker_lib/constants/map_codes.hpp"

/**
 * This patch optimizes a specific map in Swamp Shrine (room with falling spikeballs, buttons & platforms) which
 * is known to be very laggy.
 */
class PatchOptimizeSwampShrineMap : public GamePatch
{
private:
    static constexpr uint32_t ADDR_SPIKEBALL_1 = 0xFF5480;
    static constexpr uint32_t ADDR_SPIKEBALL_2 = ADDR_SPIKEBALL_1 + 0x80;
    static constexpr uint32_t ADDR_SPIKEBALL_3 = ADDR_SPIKEBALL_2 + 0x80;
    static constexpr uint32_t ADDR_SPIKEBALL_4 = ADDR_SPIKEBALL_3 + 0x80;

public:
    void alter_world(World& world) override
    {
        // Remove buttons behavior
        world.map(MAP_SWAMP_SHRINE_23)->entity(5)->behavior_id(0);
        world.map(MAP_SWAMP_SHRINE_23)->entity(7)->behavior_id(0);
        world.map(MAP_SWAMP_SHRINE_23)->entity(9)->behavior_id(0);
        world.map(MAP_SWAMP_SHRINE_23)->entity(11)->behavior_id(0);

        // Remove platforms behavior
        world.map(MAP_SWAMP_SHRINE_23)->entity(6)->behavior_id(0);
        world.map(MAP_SWAMP_SHRINE_23)->entity(8)->behavior_id(0);
        world.map(MAP_SWAMP_SHRINE_23)->entity(10)->behavior_id(0);
        world.map(MAP_SWAMP_SHRINE_23)->entity(12)->behavior_id(0);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        uint32_t func_disable_spikeballs_progessively = inject_func_disable_spikeballs_progessively(rom);
        uint32_t func_handle_buttons = inject_func_handle_buttons(rom);
        uint32_t func_handle_platforms = inject_func_handle_platforms(rom);

        md::Code func;
        func.movem_to_stack({ reg_D0 }, { reg_A0 });
        {
            func.jsr(func_disable_spikeballs_progessively);
            func.jsr(func_handle_buttons);
            func.jsr(func_handle_platforms);
        }
        func.label("ret");
        func.movem_from_stack({ reg_D0 }, { reg_A0 });
        func.rts();

        uint32_t update_func_addr = rom.inject_code(func);
        world.map(MAP_SWAMP_SHRINE_23)->map_update_addr(update_func_addr);
    }

private:
    static uint32_t inject_func_handle_buttons(md::ROM& rom)
    {
        md::Code func;

        func.movew(addr_(0xFF5430), reg_D0);    // D0 = entity underneath player

        func.btst(0, addr_(0xFF1001));
        func.bne("not_on_btn_1");
        func.cmpiw(0x0300, reg_D0);
        func.bne("not_on_btn_1");
        {
            func.bset(0, addr_(0xFF1001));      // Set the flag to tell the platform to raise
            func.trap(0, { 0x00, 0x6C });       // Play switch sound
            func.moveb(4, addr_(0xFF5725));     // Change the button sprite to "pressed"
            func.moveb(0x80, addr_(0xFF5748));  // Force sprite update
        }
        func.label("not_on_btn_1");

        func.btst(1, addr_(0xFF1001));
        func.bne("not_on_btn_2");
        func.cmpiw(0x0400, reg_D0);
        func.bne("not_on_btn_2");
        {
            func.bset(1, addr_(0xFF1001));      // Set the flag to tell the platform to raise
            func.trap(0, { 0x00, 0x6C });       // Play switch sound
            func.moveb(4, addr_(0xFF5825));     // Change the button sprite to "pressed"
            func.moveb(0x80, addr_(0xFF5848));  // Force sprite update
        }
        func.label("not_on_btn_2");

        func.btst(2, addr_(0xFF1001));
        func.bne("not_on_btn_3");
        func.cmpiw(0x0500, reg_D0);
        func.bne("not_on_btn_3");
        {
            func.bset(2, addr_(0xFF1001));      // Set the flag to tell the platform to raise
            func.trap(0, { 0x00, 0x6C });       // Play switch sound
            func.moveb(4, addr_(0xFF5925));     // Change the button sprite to "pressed"
            func.moveb(0x80, addr_(0xFF5948));  // Force sprite update
        }
        func.label("not_on_btn_3");

        func.btst(3, addr_(0xFF1001));
        func.bne("not_on_btn_4");
        func.cmpiw(0x0600, reg_D0);
        func.bne("not_on_btn_4");
        {
            func.bset(3, addr_(0xFF1001));      // Set the flag to tell the platform to raise
            func.trap(0, { 0x00, 0x6C });       // Play switch sound
            func.moveb(4, addr_(0xFF5A25));     // Change the button sprite to "pressed"
            func.moveb(0x80, addr_(0xFF5A48));  // Force sprite update
        }
        func.label("not_on_btn_4");
        func.label("ret");
        func.rts();

        return rom.inject_code(func);
    }

    static uint32_t inject_func_handle_platforms(md::ROM& rom)
    {
        md::Code func;

        func.btst(0, addr_(0xFF1001));
        func.beq("platform_1_done");
        func.movew(addr_(0xFF5792), reg_D0); // D0 = platform Z
        func.cmpiw(0x001F, reg_D0);
        func.bgt("platform_1_done");
        {
            func.addqb(1, reg_D0);
            func.movew(reg_D0, addr_(0xFF5792));
            func.addqb(7, reg_D0);
            func.movew(reg_D0, addr_(0xFF57D4));
        }
        func.label("platform_1_done");

        func.btst(1, addr_(0xFF1001));
        func.beq("platform_2_done");
        func.movew(addr_(0xFF5892), reg_D0); // D0 = platform Z
        func.cmpiw(0x003F, reg_D0);
        func.bgt("platform_2_done");
        {
            func.addqb(1, reg_D0);
            func.movew(reg_D0, addr_(0xFF5892));
            func.addqb(7, reg_D0);
            func.movew(reg_D0, addr_(0xFF58D4));
        }
        func.label("platform_2_done");

        func.btst(2, addr_(0xFF1001));
        func.beq("platform_3_done");
        func.movew(addr_(0xFF5992), reg_D0); // D0 = platform Z
        func.cmpiw(0x005F, reg_D0);
        func.bgt("platform_3_done");
        {
            func.addqb(1, reg_D0);
            func.movew(reg_D0, addr_(0xFF5992));
            func.addqb(7, reg_D0);
            func.movew(reg_D0, addr_(0xFF59D4));
        }
        func.label("platform_3_done");

        func.btst(3, addr_(0xFF1001));
        func.beq("platform_4_done");
        func.movew(addr_(0xFF5A92), reg_D0); // D0 = platform Z
        func.cmpiw(0x007F, reg_D0);
        func.bgt("platform_4_done");
        {
            func.addqb(1, reg_D0);
            func.movew(reg_D0, addr_(0xFF5A92));
            func.addqb(7, reg_D0);
            func.movew(reg_D0, addr_(0xFF5AD4));
        }
        func.label("platform_4_done");
        func.rts();

        return rom.inject_code(func);
    }

    static uint32_t inject_func_disable_spikeballs_progessively(md::ROM& rom)
    {
        md::Code func;

        func.moveq(0x35, reg_D0);
        func.moveb(reg_D0, addr_(ADDR_SPIKEBALL_1));
        func.moveb(reg_D0, addr_(ADDR_SPIKEBALL_2));
        func.moveb(reg_D0, addr_(ADDR_SPIKEBALL_3));
        func.moveb(reg_D0, addr_(ADDR_SPIKEBALL_4));

        func.moveb(addr_(0xFF5400), reg_D0); // D0 = player's position

        // X <= 2C --> disable first spikeball if it is on ground
        func.cmpib(0x2C, reg_D0);
        func.bgt("ret");
        func.cmpib(0x20, addr_(ADDR_SPIKEBALL_1 + 0x13));
        func.bne("s1_not_on_ground");
        {
            func.moveb(0x7F, addr_(ADDR_SPIKEBALL_1));
        }
        func.label("s1_not_on_ground");

        // X <= 27 --> disable second spikeball if it is on ground
        func.cmpib(0x27, reg_D0);
        func.bgt("ret");
        func.cmpib(0x40, addr_(ADDR_SPIKEBALL_2 + 0x13));
        func.bne("s2_not_on_ground");
        {
            func.moveb(0x7F, addr_(ADDR_SPIKEBALL_2));
        }
        func.label("s2_not_on_ground");

        // X <= 22 --> disable third spikeball if it is on ground
        func.cmpib(0x22, reg_D0);
        func.bgt("ret");
        func.cmpib(0x60, addr_(ADDR_SPIKEBALL_3 + 0x13));
        func.bne("ret");
        {
            func.moveb(0x7F, addr_(ADDR_SPIKEBALL_3));
        }
        func.label("ret");
        func.rts();

        return rom.inject_code(func);
    }
};
