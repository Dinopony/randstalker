#pragma once

#include <landstalker_lib/model/world.hpp>
#include <landstalker_lib/model/entity_type.hpp>
#include <landstalker_lib/tools/sprite.hpp>
#include <landstalker_lib/md_tools.hpp>

void extract_nigel_sprites(md::ROM& rom, const World& world)
{
    EntityLowPalette palette_low = world.entity_types().at(0)->low_palette();
    EntityHighPalette palette_high = world.entity_types().at(0)->high_palette();

    ColorPalette<16> palette {};
    palette[0] = { 0, 0, 0 }; // Transparent
    palette[1] = { 0xC0, 0xC0, 0xC0 };
    for(size_t i=0 ; i<palette_low.size() ; ++i)
        palette[i+2] = palette_low[i];
    for(size_t i=0 ; i<palette_high.size() ; ++i)
        palette[i+2+palette_low.size()] = palette_high[i];
    palette[15] = { 0, 0, 0 }; // Black

    uint32_t base_addr = 0x120800;
    for(uint8_t frame_id = 0 ; frame_id < 73 ; ++frame_id)
    {
        uint32_t frame_addr = rom.get_long(base_addr + (frame_id * 0x4));
        Sprite frame = Sprite::decode_from(rom.iterator_at(frame_addr));
        frame.write_to_png("./sprites/nigel/" + std::to_string(frame_id) + ".png", palette);
    }
}