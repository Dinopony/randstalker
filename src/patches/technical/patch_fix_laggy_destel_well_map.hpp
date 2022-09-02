#pragma once

#include <landstalker_lib/patches/game_patch.hpp>
#include <landstalker_lib/model/map.hpp>
#include <landstalker_lib/constants/map_codes.hpp>

class PatchFixLaggyDestelWellMap : public GamePatch
{
public:
    void alter_world(World& world) override
    {
        world.map(MAP_DESTEL_WELL_274)->remove_entity(8);
        world.map(MAP_DESTEL_WELL_274)->remove_entity(7);
        world.map(MAP_DESTEL_WELL_274)->remove_entity(6);
        world.map(MAP_DESTEL_WELL_274)->remove_entity(5);
        world.map(MAP_DESTEL_WELL_274)->remove_entity(2);
        world.map(MAP_DESTEL_WELL_274)->remove_entity(0);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        uint32_t func_process_entity = inject_func_process_entity(rom);

        md::Code func;
        func.movem_to_stack({ reg_D0, reg_D1 }, { reg_A0 });
        {
            func.moveb(addr_(0xFF1001), reg_D1);    // Store old flag state in D1
            func.clrb(addr_(0xFF1001));

            func.lea(0xFF5400, reg_A0);
            func.label("loop");
            {
                func.cmpa(0xFF5480, reg_A0); // No need to check anything with the door
                func.beq("next_entity");
                func.cmpa(0xFF5B00, reg_A0); // End of array
                func.beq("ret");
                func.jsr(func_process_entity);
                func.label("next_entity");
                func.adda(0x80, reg_A0);
            }
            func.bra("loop");
        }
        func.label("ret");
        // Compare old flag state to new flag state: if any new holes have been filled, play a switch sound
        func.notb(reg_D1);
        func.andib(0x0F, reg_D1);
        func.andb(addr_(0xFF1001), reg_D1);
        func.tstb(reg_D1);
        func.beq("no_sound");
        {
            func.trap(0, { 0x00, 0x6C });
        }
        func.label("no_sound");
        func.movem_from_stack({ reg_D0, reg_D1 }, { reg_A0 });
        func.rts();

        uint32_t update_func_addr = rom.inject_code(func);
        world.map(MAP_DESTEL_WELL_274)->map_update_addr(update_func_addr);
    }

private:
    static uint32_t inject_func_process_entity(md::ROM& rom)
    {
        md::Code func;

        func.moveb(addr_(reg_A0, 0x13), reg_D0); // D0.b = height
        func.cmpib(0x90, reg_D0);
        func.bne("no_door_trigger");
        {
            // Test door trigger X (must be between 299 and 2A7)
            {
                func.movew(addr_(reg_A0, 0x18), reg_D0); // min X
                func.cmpiw(0x299, reg_D0);
                func.blt("invalid_min_x");
                func.cmpiw(0x2A7+1, reg_D0);
                func.blt("valid_x");
            }
            func.label("invalid_min_x");
            {
                func.movew(addr_(reg_A0, 0x1A), reg_D0); // max X
                func.cmpiw(0x299, reg_D0);
                func.blt("next_entity");
                func.cmpiw(0x2A7, reg_D0);
                func.bgt("next_entity");
            }

            // Test door trigger Y (must be between 231 and 24F)
            func.label("valid_x");
            {
                func.movew(addr_(reg_A0, 0x1C), reg_D0); // min Y
                func.cmpiw(0x231, reg_D0);
                func.blt("invalid_min_y");
                func.cmpiw(0x24F+1, reg_D0);
                func.blt("door_trigger_hit");
            }
            func.label("invalid_min_y");
            {
                func.movew(addr_(reg_A0, 0x1E), reg_D0); // max Y
                func.cmpiw(0x231, reg_D0);
                func.blt("next_entity");
                func.cmpiw(0x24F, reg_D0);
                func.bgt("next_entity");
            }
            func.label("door_trigger_hit");
            func.bset(4, addr_(0xFF1001));
            func.rts();
        }
        func.label("no_door_trigger");
        func.cmpib(0x80, reg_D0);
        func.bne("next_entity");
        {
            // Test holes (2121, 2126, 2621, 2626 at height 0080)
            func.movew(addr_(reg_A0), reg_D0); // D0.w = pos
            func.cmpiw(0x2621, reg_D0);
            func.bne("not_hole_1");
            {
                func.bset(0, addr_(0xFF1001));
                func.rts();
            }
            func.label("not_hole_1");

            func.cmpiw(0x2126, reg_D0);
            func.bne("not_hole_2");
            {
                func.bset(1, addr_(0xFF1001));
                func.rts();
            }
            func.label("not_hole_2");

            func.cmpiw(0x2121, reg_D0);
            func.bne("not_hole_3");
            {
                func.bset(2, addr_(0xFF1001));
                func.rts();
            }
            func.label("not_hole_3");

            func.cmpiw(0x2626, reg_D0);
            func.bne("next_entity");
            {
                func.bset(3, addr_(0xFF1001));
            }
        }
        func.label("next_entity");
        func.rts();

        return rom.inject_code(func);
    }
};
