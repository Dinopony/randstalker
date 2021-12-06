#include <md_tools.hpp>

#include "../../world_model/world.hpp"
#include "../../constants/offsets.hpp"
#include "../../tools/byte_array.hpp"

static uint32_t inject_function_darken_palette(md::ROM& rom)
{
    // ----------------------------------------
    // Function to darken the color palette currently used to draw the map
    // Input :  D0 = AND mask to apply to palette
    md::Code func_darken_palette;

    func_darken_palette.movem_to_stack({ reg_D1, reg_D2 }, { reg_A0 });
    func_darken_palette.lea(0xFF0080, reg_A0);
    func_darken_palette.movew(0x20, reg_D1);
    func_darken_palette.label("loop");
        func_darken_palette.movew(addr_(reg_A0), reg_D2);
        func_darken_palette.andw(reg_D0, reg_D2);
        func_darken_palette.movew(reg_D2, addr_(reg_A0));
        func_darken_palette.adda(0x2, reg_A0);
        func_darken_palette.dbra(reg_D1, "loop");
    func_darken_palette.movem_from_stack({ reg_D1, reg_D2 }, { reg_A0 });
    func_darken_palette.rts();

    return rom.inject_code(func_darken_palette);
}

static uint32_t inject_dark_rooms_table(md::ROM& rom, const World& world)
{
    // Inject dark rooms as a data block
    ByteArray bytes;
    for (uint16_t map_id : world.dark_maps())
        bytes.add_word(map_id);
    bytes.add_word(0xFFFF);

    return rom.inject_bytes(bytes);
}

static void replace_function_check_lantern(md::ROM& rom, uint32_t darken_palette, uint32_t dark_rooms_table)
{
    // ----------------------------------------
    // Function to check if the current room is supposed to be dark, and process differently 
    // depending on whether or not we own the lantern
    md::Code func_check_lantern;

    func_check_lantern.lea(dark_rooms_table, reg_A0);
    func_check_lantern.label("loop_start");
    func_check_lantern.movew(addr_(reg_A0), reg_D0);
    func_check_lantern.bmi(14);
        func_check_lantern.cmpw(addr_(0xFF1204), reg_D0);
        func_check_lantern.bne(10);
            // We are in a dark room
            func_check_lantern.movem_to_stack({ reg_D0 }, {});
            func_check_lantern.btst(0x1, addr_(0xFF104D));
            func_check_lantern.bne(3);
                // Dark room with no lantern ===> darken the palette
                func_check_lantern.movew(0x0000, reg_D0);
                func_check_lantern.bra(2);

                // Dark room with lantern ===> use lantern palette
                func_check_lantern.movew(0x0CCC, reg_D0);
            func_check_lantern.jsr(darken_palette);
            func_check_lantern.movem_from_stack({ reg_D0 }, {});
            func_check_lantern.rts();
        func_check_lantern.addql(0x2, reg_A0);
        func_check_lantern.bra("loop_start");
    func_check_lantern.clrb(reg_D7);
    func_check_lantern.rts();

    rom.set_code(0x87BE, func_check_lantern);
}

static void replace_function_change_map_palette(md::ROM& rom)
{
    // ----------------------------------------
    // Function to change the palette used on map transition (already exist in OG, slightly modified)
    md::Code func_change_map_palette;

    func_change_map_palette.cmpb(addr_(0xFF112F), reg_D4);
    func_change_map_palette.beq(12);
        func_change_map_palette.moveb(reg_D4, addr_(0xFF112F));
        func_change_map_palette.movel(addr_(offsets::MAP_PALETTES_TABLE_POINTER), reg_A0);
        func_change_map_palette.mulu(bval_(0x1A), reg_D4);
        func_change_map_palette.adda(reg_D4, reg_A0);
        func_change_map_palette.lea(0xFF0084, reg_A1);
        func_change_map_palette.movew(0x000C, reg_D0);
        func_change_map_palette.jsr(0x96A);
        func_change_map_palette.clrw(addr_(0xFF0080));
        func_change_map_palette.movew(0x0CCC, addr_(0xFF0082));
        func_change_map_palette.clrw(addr_(0xFF009E));
        func_change_map_palette.jsr(0x87BE);
    func_change_map_palette.rts();

    rom.set_code(0x2D64, func_change_map_palette);
}

/**
 * Make the lantern a flexible item, by actually being able to impact a predefined
 * table of "dark maps" and turning screen to pitch black if it's not owned.
 */
void alter_lantern_handling(md::ROM& rom, const World& world)
{
    uint32_t dark_rooms_table = inject_dark_rooms_table(rom, world);
    uint32_t darken_palette = inject_function_darken_palette(rom);
    replace_function_check_lantern(rom, darken_palette, dark_rooms_table);
    replace_function_change_map_palette(rom);

    // ----------------------------------------
    // Pseudo-function used to extend the global palette init function, used to call the lantern check
    // after initing all palettes (both map & entities)
    md::Code ext_init_palette;

    ext_init_palette.movew(0x0CCC, addr_(0xFF00A2));
    ext_init_palette.add_bytes(rom.data_chunk(0x19520, 0x19526)); // Add the jsr that was removed for injection
    ext_init_palette.jsr(0x87BE);
    ext_init_palette.rts();
    uint32_t addr = rom.inject_code(ext_init_palette);
    rom.set_code(0x19520, md::Code().jsr(addr));

    // ----------------------------------------
    // Replace the "dark room" palette from King Nole's Labyrinth by the lit room palette
    for (uint8_t i = 0; i < 0x1A; ++i)
        rom.set_byte(offsets::KNL_DARK_ROOM_PALETTE + i, rom.get_byte(offsets::KNL_LIT_ROOM_PALETTE + i));
}