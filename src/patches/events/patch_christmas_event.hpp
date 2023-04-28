#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <landstalker-lib/model/map.hpp>
#include <landstalker-lib/model/world.hpp>
#include <landstalker-lib/constants/map_codes.hpp>
#include <landstalker-lib/tools/sprite.hpp>

#include "../../assets/christmas_moneybag.bin.hxx"

class PatchChristmasEvent : public GamePatch
{
private:
    ColorPalette<16> _title_palette;
    Sprite _moneybag_sprite;

public:
    void load_from_rom(const md::ROM& rom) override
    {
        _title_palette = ColorPalette<16>::from_rom(rom, 0x3DECA);

        constexpr uint32_t MONEYBAG_SPRITE_ADDR = 0x151CC4;
        _moneybag_sprite = Sprite::decode_from(rom.iterator_at(MONEYBAG_SPRITE_ADDR));
    }

    void alter_world(World& world) override
    {
        // Change map palettes for a more snowy mood
        std::vector<uint16_t> PALETTES_TO_PROCESS = {
                MAP_ROUTE_TO_RYUMA_1,
                MAP_ROUTE_GUMI_RYUMA_TIBOR_CROSSROADS,
                MAP_RYUMA_EXTERIOR,
                MAP_ROUTE_GUMI_RYUMA_HELGA_SWAMP,
                MAP_ROUTE_GUMI_RYUMA_MERCATOR_GATES,
                MAP_MERCATOR_CASTLE_EXTERIOR,
                MAP_DESTEL_EXTERIOR,
                MAP_ROUTE_AFTER_DESTEL_VIEWPOINT,
                MAP_MERCATOR_DOCKS_LIGHTHOUSE_FIXED_VARIANT
        };

        for(uint16_t map_id : PALETTES_TO_PROCESS)
        {
            MapPalette* palette = world.map(map_id)->palette();
            palette->multiply(1.2);
            palette->desaturate(0.4);
            palette->at(3) = Color(0xA0, 0xA0, 0xC0);
            palette->at(4) = Color(0xC0, 0xC0, 0xE0);
            palette->at(5) = Color(0xE0, 0xE0, 0xE0);
        }

        MapPalette* palette = world.map(MAP_MERCATOR_DOCKS_DARK_VARIANT)->palette();
        palette->at(3) = Color(0xA0, 0xA0, 0xC0).multiply(0.7);
        palette->at(4) = Color(0xC0, 0xC0, 0xE0).multiply(0.7);
        palette->at(5) = Color(0xE0, 0xE0, 0xE0).multiply(0.7);

        // AUTUMN PALETTE
        // palette->at(3) = Color(0x60, 0x20, 0x00);
        // palette->at(4) = Color(0xA0, 0x40, 0x00);
        // palette->at(5) = Color(0xC0, 0x80, 0x20);
    }

    void alter_rom(md::ROM& rom) override
    {
        // Change title screen palette
        _title_palette[8] = Color::from_bgr_word(0x22C);
        _title_palette[9] = Color::from_bgr_word(0x028);
        _title_palette[10] = Color::from_bgr_word(0x226);
        _title_palette[11] = Color::from_bgr_word(0x24E);
        rom.set_bytes(0x3DECA, _title_palette.to_bytes());
    }

    void inject_data(md::ROM& rom, World& world) override
    {
        // Change money bags into presents
        Sprite edited_moneybag = Sprite(CHRISTMAS_MONEYBAG, CHRISTMAS_MONEYBAG_SIZE, _moneybag_sprite.subsprites());
        uint32_t new_moneybag_sprite_addr = rom.inject_bytes(edited_moneybag.encode());
        rom.set_long(0x121A3C, new_moneybag_sprite_addr);
    }
};
