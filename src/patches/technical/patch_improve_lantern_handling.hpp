#pragma once

#include <landstalker-lib/patches/game_patch.hpp>

#include <landstalker-lib/model/world.hpp>
#include <landstalker-lib/model/map.hpp>
#include <landstalker-lib/model/item.hpp>
#include <landstalker-lib/constants/offsets.hpp>
#include <landstalker-lib/constants/item_codes.hpp>
#include <landstalker-lib/exceptions.hpp>

/**
 * Make the lantern a flexible item, by actually being able to impact a predefined
 * table of "dark maps" and turning screen to pitch black if it's not owned.
 */
class PatchImproveLanternHandling : public GamePatch
{
private:
    uint32_t _dark_rooms_table_addr = 0xFFFFFFFF;

public:
    void clear_space_in_rom(md::ROM& rom) override
    {
        // Remove all code related to old lantern usage
        rom.mark_empty_chunk(0x87AA, 0x8832);
    }

    void alter_world(World& world) override
    {
        world.item(ITEM_LANTERN)->pre_use_address(0);

        // Replace the "dark room" palette from King Nole's Labyrinth by the lit room palette
        MapPalette* lit_knl_palette = world.map_palettes().at(39);
        MapPalette* dark_knl_palette = world.map_palettes().at(40);

        for(auto& [map_id, map] : world.maps())
            if(map->palette() == dark_knl_palette)
                map->palette(lit_knl_palette);
    }

    void inject_data(md::ROM& rom, World& world) override
    {
        // Inject dark rooms as a data block
        ByteArray bytes;
        for (uint16_t map_id : world.dark_maps())
            bytes.add_word(map_id);
        bytes.add_word(0xFFFF);

        _dark_rooms_table_addr = rom.inject_bytes(bytes);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        uint32_t and_words = inject_func_and_words(rom);
        uint32_t darken_palettes = inject_func_darken_palettes(rom, and_words);
        uint32_t dim_palettes = inject_func_dim_palettes(rom, and_words);
        uint32_t darken_if_missing_lantern = inject_func_darken_if_missing_lantern(rom, darken_palettes, dim_palettes);

        simplify_function_change_map_palette(rom, darken_if_missing_lantern);

        // Remove the fixed addition of white in the InitPalettes function to prevent parasitic whites to appear when
        // leaving menu or loading a saved game
        rom.set_code(0x8FFA, md::Code().nop(4));
        rom.set_code(0x9006, md::Code().nop(2));

        // ----------------------------------------
        // Hook procedure used to call the lantern check after initing all palettes (both map & entities)
        md::Code proc_init_palettes;
        proc_init_palettes.add_bytes(rom.get_bytes(0x19520, 0x19526)); // Add the jsr that was removed for injection
        proc_init_palettes.jsr(darken_if_missing_lantern);
        proc_init_palettes.rts();
        uint32_t addr = rom.inject_code(proc_init_palettes);
        rom.set_code(0x19520, md::Code().jsr(addr));
    }

private:
    /**
     * Apply an AND mask (stored in D0) on D1 words starting from A0
     * @param rom
     * @return
     */
    static uint32_t inject_func_and_words(md::ROM& rom)
    {
        md::Code func;
        func.movem_to_stack({ reg_D2 }, {});
        {
            func.label("loop");
            {
                func.movew(addr_(reg_A0), reg_D2);
                func.andw(reg_D0, reg_D2);
                func.movew(reg_D2, addr_postinc_(reg_A0));
            }
            func.dbra(reg_D1, "loop");
        }
        func.movem_from_stack({ reg_D2 }, {});
        func.rts();
        return rom.inject_code(func);
    }

    static uint32_t inject_func_darken_palettes(md::ROM& rom, uint32_t func_and_words)
    {
        // Darken palettes with AND filter contained in D0
        md::Code func;
        func.movem_to_stack({ reg_D1 }, { reg_A0 });
        {
            // Turn palette 0 & 1 pitch black
            func.movew(0x0000, reg_D0);
            func.lea(0xFF0080, reg_A0);
            func.movew(32, reg_D1);
            func.jsr(func_and_words);

            // Turn lower half of 3 pitch black
            func.lea(0xFF00E0, reg_A0);
            func.movew(8, reg_D1);
            func.jsr(func_and_words);

            // Darken palette 2 (Nigel, chests, platforms...)
            // func.lea(0xFF00C0, reg_A0);
            // func.movew(16, reg_D1);
            // func.movew(0x0444, reg_D0);
            // func.jsr(func_and_words);
        }
        func.movem_from_stack({ reg_D1 }, { reg_A0 });
        func.rts();
        return rom.inject_code(func);
    }

    static uint32_t inject_func_dim_palettes(md::ROM& rom, uint32_t func_and_words)
    {
        // Darken palettes with AND filter contained in D0
        md::Code func;
        func.movem_to_stack({ reg_D1 }, { reg_A0 });
        {
            // Slightly dim palette 0
            func.movew(0x0CCC, reg_D0);
            func.lea(0xFF0080, reg_A0);
            func.movew(16, reg_D1);
            func.jsr(func_and_words);
        }
        func.movem_from_stack({ reg_D1 }, { reg_A0 });
        func.rts();
        return rom.inject_code(func);
    }

    uint32_t inject_func_darken_if_missing_lantern(md::ROM& rom, uint32_t func_darken_palettes, uint32_t func_dim_palettes) const
    {
        md::Code func;
        func.movem_to_stack({ reg_D0 }, { reg_A0 });
        {
            // Put back white color where appropriate in all palettes to ensure it's there before an eventual darkening
            func.movew(0x0CCC, reg_D0);
            func.movew(reg_D0, addr_(0xFF0082));
            func.movew(reg_D0, addr_(0xFF00A2));
            func.movew(reg_D0, addr_(0xFF00E2));

            func.lea(_dark_rooms_table_addr, reg_A0);

            // Try to find current room in dark rooms list
            func.label("loop_start");
            {
                func.movew(addr_postinc_(reg_A0), reg_D0);
                func.bmi("return"); // If negative, it means we reached end of table (0xFFFF)
                func.cmpw(addr_(0xFF1204), reg_D0);
                func.beq("current_room_is_dark");
            }
            func.bra("loop_start");

            // We are in a dark room
            func.label("current_room_is_dark");
            func.moveb(0xFE, addr_(0xFF112F));
            func.btst(0x1, addr_(0xFF104D));
            func.bne("owns_lantern");
            {
                // Dark room without lantern ===> make palettes fully black
                func.jsr(func_darken_palettes);
                func.bra("return");
            }
            func.label("owns_lantern");
            // Dark room with lantern ===> very slightly dim the palettes
            func.jsr(func_dim_palettes);
        }
        func.label("return");
        func.movem_from_stack({ reg_D0 }, { reg_A0 });
        func.rts();

        return rom.inject_code(func);
    }

    static void simplify_function_change_map_palette(md::ROM& rom, uint32_t func_darken_if_missing_lantern)
    {
        md::Code func;
        func.cmpb(addr_(0xFF112F), reg_D4);
        func.beq("return");
        {
            func.moveb(reg_D4, addr_(0xFF112F));
            func.movel(addr_(offsets::MAP_PALETTES_TABLE_POINTER), reg_A0);
            func.mulu(bval_(26), reg_D4);
            func.adda(reg_D4, reg_A0);
            func.lea(0xFF0084, reg_A1);
            func.movew(0x000C, reg_D0);
            func.jsr(0x96A);
            func.clrw(addr_(0xFF009E));
            func.clrw(addr_(0xFF0080));
            func.jsr(func_darken_if_missing_lantern);
        }
        func.label("return");
        func.rts();

        rom.set_code(0x2D64, func);
        if(func.size() > 106)
            throw LandstalkerException("func_change_map_palette is bigger than the original one");
    }
};
