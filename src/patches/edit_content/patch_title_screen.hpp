#pragma once

#include <landstalker-lib/patches/game_patch.hpp>

#include "../../assets/titlescreen/title1_tiles.lz77.hxx"
#include "../../assets/titlescreen/title1_layout.rle.hxx"
#include "../../assets/titlescreen/title2_tiles.lz77.hxx"
#include "../../assets/titlescreen/title2_layout.rle.hxx"
#include "../../assets/titlescreen/title3_tiles.lz77.hxx"
#include "../../assets/titlescreen/title3_layout.rle.hxx"

class PatchTitleScreen : public GamePatch
{
private:
    uint32_t _title1_tiles_addr = 0xFFFFFFFF;
    uint32_t _title1_layout_addr = 0xFFFFFFFF;
    uint32_t _title2_tiles_addr = 0xFFFFFFFF;
    uint32_t _title2_layout_addr = 0xFFFFFFFF;
    uint32_t _title3_tiles_addr = 0xFFFFFFFF;
    uint32_t _title3_layout_addr = 0xFFFFFFFF;

public:
    void alter_rom(md::ROM& rom) override
    {
        // Fix fade-in
        rom.set_byte(0x398F1, 0x14); // Start X
        rom.set_byte(0x398F3, 0x0C); // Start Y
        rom.set_byte(0x398F9, 0x0E); // Width
        rom.set_byte(0x398FB, 0x08); // Height
    }

    void clear_space_in_rom(md::ROM& rom) override
    {
        // Empty all the data that is going to be reinjected with edits
        rom.mark_empty_chunk(0x39ED8, 0x3DECA);
    }

    void inject_data(md::ROM& rom, World& world) override
    {
        _title1_tiles_addr = rom.inject_bytes(TITLE1_TILES, TITLE1_TILES_SIZE);
        _title1_layout_addr = rom.inject_bytes(TITLE1_LAYOUT, TITLE1_LAYOUT_SIZE);
        _title2_tiles_addr = rom.inject_bytes(TITLE2_TILES, TITLE2_TILES_SIZE);
        _title2_layout_addr = rom.inject_bytes(TITLE2_LAYOUT, TITLE2_LAYOUT_SIZE);
        _title3_tiles_addr = rom.inject_bytes(TITLE3_TILES, TITLE3_TILES_SIZE);
        _title3_layout_addr = rom.inject_bytes(TITLE3_LAYOUT, TITLE3_LAYOUT_SIZE);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        rom.set_code(0x3985E, md::Code().jsr(inject_pointer_func(rom, _title1_tiles_addr)).nop(2));
        rom.set_code(0x39872, md::Code().jsr(inject_pointer_func(rom, _title1_layout_addr)).nop(2));
        rom.set_code(0x39892, md::Code().jsr(inject_pointer_func(rom, _title2_tiles_addr)).nop(2));
        rom.set_code(0x398A6, md::Code().jsr(inject_pointer_func(rom, _title2_layout_addr)).nop(2));
        rom.set_code(0x398C6, md::Code().jsr(inject_pointer_func(rom, _title3_tiles_addr)).nop(2));
        rom.set_code(0x398DA, md::Code().jsr(inject_pointer_func(rom, _title3_layout_addr)).nop(2));
    }

private:
    static uint32_t inject_pointer_func(md::ROM& rom, uint32_t pointed_addr)
    {
        md::Code pointer_func;
        {
            pointer_func.lea(pointed_addr, reg_A0);
            pointer_func.lea(0x00FF2C00, reg_A1);
        }
        pointer_func.rts();

        return rom.inject_code(pointer_func);
    }
};
