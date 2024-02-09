#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <landstalker-lib/model/map.hpp>
#include <landstalker-lib/model/world.hpp>
#include <landstalker-lib/constants/map_codes.hpp>
#include <landstalker-lib/tools/sprite.hpp>

#include "../../assets/christmas_moneybag.bin.hxx"

class PatchSeasonTheme : public GamePatch
{
private:
    Season _season;

public:
    PatchSeasonTheme(Season season) : _season(season) {}

    void alter_world(World& world) override
    {
        std::vector<uint16_t> PALETTES_TO_PROCESS = {
                MAP_ROUTE_TO_RYUMA_1,
                MAP_ROUTE_GUMI_RYUMA_TIBOR_CROSSROADS,
                MAP_RYUMA_EXTERIOR,
                MAP_ROUTE_GUMI_RYUMA_HELGA_SWAMP,
                MAP_ROUTE_GUMI_RYUMA_MERCATOR_GATES,
                MAP_MERCATOR_CASTLE_EXTERIOR,
                MAP_DESTEL_EXTERIOR,
                MAP_ROUTE_AFTER_DESTEL_VIEWPOINT,
                MAP_MERCATOR_DOCKS_LIGHTHOUSE_FIXED_VARIANT,
                MAP_MERCATOR_DOCKS_DARK_VARIANT
        };

        for(uint16_t map_id : PALETTES_TO_PROCESS)
        {
            MapPalette* palette = world.map(map_id)->palette();
            edit_palette(palette, (map_id == MAP_MERCATOR_DOCKS_DARK_VARIANT));
        }
    }

    void alter_rom(md::ROM& rom) override
    {
        if(_season == Season::WINTER)
        {
            // Change title screen palette
            ColorPalette<16> title_palette = ColorPalette<16>::from_rom(rom, 0x3DECA);
            title_palette[8] = Color::from_bgr_word(0x22C);
            title_palette[9] = Color::from_bgr_word(0x028);
            title_palette[10] = Color::from_bgr_word(0x226);
            title_palette[11] = Color::from_bgr_word(0x24E);
            rom.set_bytes(0x3DECA, title_palette.to_bytes());
        }
    }

    void inject_data(md::ROM& rom, World& world) override
    {
        if(_season == Season::WINTER)
        {
            // Change money bags into presents
            constexpr uint32_t MONEYBAG_SPRITE_ADDR = 0x151CC4;
            Sprite moneybag_sprite = Sprite::decode_from(rom.iterator_at(MONEYBAG_SPRITE_ADDR));
            Sprite edited_moneybag = Sprite(CHRISTMAS_MONEYBAG, CHRISTMAS_MONEYBAG_SIZE, moneybag_sprite.subsprites());
            uint32_t new_moneybag_sprite_addr = rom.inject_bytes(edited_moneybag.encode());
            rom.set_long(0x121A3C, new_moneybag_sprite_addr);
        }
    }

private:
    void edit_palette(MapPalette* palette, bool is_dark_docks)
    {
        if(_season == Season::WINTER)
        {
            // Change map palettes for a more snowy mood
            if(!is_dark_docks)
            {
                palette->multiply(1.2);
                palette->desaturate(0.4);
            }
            palette->at(3) = Color(0xA0, 0xA0, 0xC0);
            palette->at(4) = Color(0xC0, 0xC0, 0xE0);
            palette->at(5) = Color(0xE0, 0xE0, 0xE0);
        }
        else if(_season == Season::AUTUMN)
        {
            palette->at(3) = Color(0x60, 0x20, 0x00);
            palette->at(4) = Color(0xA0, 0x40, 0x00);
            palette->at(5) = Color(0xC0, 0x80, 0x20);
        }
        else if(_season == Season::SUMMER)
        {
            palette->at(3) = Color(0x60, 0x40, 0x20).desaturate(0.2);
            palette->at(4) = Color(0xA0, 0x80, 0x00).desaturate(0.2);
            palette->at(5) = Color(0xC0, 0xC0, 0x40).desaturate(0.2);
        }

        if(is_dark_docks)
        {
            palette->at(3) = palette->at(3).multiply(0.7);
            palette->at(4) = palette->at(4).multiply(0.7);
            palette->at(5) = palette->at(5).multiply(0.7);
        }
    }
};
