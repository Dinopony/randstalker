#include <landstalker_lib/md_tools.hpp>
#include <landstalker_lib/model/map.hpp>
#include <landstalker_lib/model/world.hpp>
#include <landstalker_lib/constants/map_codes.hpp>
#include <landstalker_lib/tools/sprite.hpp>
#include "../assets/christmas_moneybag.bin.hxx"

void christmas_event(md::ROM& rom, World& world)
{
    // Change title screen palette
    ColorPalette<16> title_palette = ColorPalette<16>::from_rom(rom, 0x3DECA);
    title_palette[8] = Color::from_bgr_word(0x22C);
    title_palette[9] = Color::from_bgr_word(0x028);
    title_palette[10] = Color::from_bgr_word(0x226);
    title_palette[11] = Color::from_bgr_word(0x24E);
    rom.set_bytes(0x3DECA, title_palette.to_bytes());

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

    // Change money bags into presents
    constexpr uint32_t MONEYBAG_SPRITE_ADDR = 0x151CC4;
    Sprite moneybag_sprite = Sprite::decode_from(rom.iterator_at(MONEYBAG_SPRITE_ADDR));
    Sprite edited_moneybag = Sprite(CHRISTMAS_MONEYBAG, CHRISTMAS_MONEYBAG_SIZE, moneybag_sprite.subsprites());

    uint32_t new_moneybag_sprite_addr = rom.inject_bytes(edited_moneybag.encode());
    rom.set_long(0x121A3C, new_moneybag_sprite_addr);
}