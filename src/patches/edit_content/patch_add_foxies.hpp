#pragma once

#include "landstalker-lib/patches/game_patch.hpp"

#include "landstalker-lib/model/entity.hpp"
#include "landstalker-lib/model/entity_type.hpp"
#include "landstalker-lib/model/map.hpp"
#include "landstalker-lib/model/world.hpp"
#include "landstalker-lib/constants/entity_type_codes.hpp"
#include "landstalker-lib/tools/byte_array.hpp"
#include "landstalker-lib/tools/vectools.hpp"
#include "landstalker-lib/constants/symbols.hpp"
#include "landstalker-lib/tools/sprite.hpp"
#include "../../logic_model/randomizer_world.hpp"
#include "../../logic_model/hint_source.hpp"

class PatchAddFoxies : public GamePatch
{
public:
    void alter_rom(md::ROM& rom) override
    {
        // Set foxes voice to the same one as Mir (replacing now unused "Kayla" and "Kayla in bath" entries)
        rom.set_byte(0x2910A, ENTITY_NPC_MAGIC_FOX);
        rom.set_byte(0x2910B, 0x6A);
        rom.set_byte(0x2910C, ENTITY_NPC_MAGIC_FOX_HIGH_PALETTE);
        rom.set_byte(0x2910D, 0x6A);

        // Set a height of 2 tiles for Foxies (like any other NPC in the game) instead of 2.5
        rom.set_byte(0x1B05D, 0x20);
    }

    void alter_world(World& world) override
    {
        RandomizerWorld& randomizer_world = reinterpret_cast<RandomizerWorld&>(world);

        std::vector<uint16_t> processed_map_ids;
        for(HintSource* hint_source : randomizer_world.used_hint_sources())
        {
            // If hint source is special, it means it's not a fox
            if(hint_source->special())
                continue;

            for(uint16_t map_id : hint_source->map_ids())
            {
                // Add the actual fox inside the map where it is meant to be
                Entity* fox = add_magic_fox(world, hint_source, map_id);
                randomizer_world.add_custom_dialogue(fox, "Foxy: " + hint_source->text());
            }
        }
    }

    void inject_data(md::ROM& rom, World& world) override
    {
        inject_altered_fox_sprites(rom, world);
    }

private:
    static void inject_altered_fox_sprites(md::ROM& rom, World& world)
    {
        // Turn the "Magic Fox" entity into a strictly low palette entity
        auto edit_sprite = [](Sprite& sprite) {
            sprite.replace_color(0x0C, 0x06); // Tunic lose one color shade
            sprite.replace_color(0x0A, 0x0F); // Eye becomes black
            sprite.replace_color(0x08, 0x07); // Move dark purple to unused slot
            sprite.replace_color(0x0B, 0x0F); // Dark grey now uses black
        };

        Sprite magic_fox_ne_sprite = Sprite::decode_from(rom.iterator_at(0x180D56));
        Sprite magic_fox_sw_sprite = Sprite::decode_from(rom.iterator_at(0x181038));
        rom.mark_empty_chunk(0x180D56, 0x182F64);

        edit_sprite(magic_fox_ne_sprite);
        uint32_t sprite_ne_addr = rom.inject_bytes(magic_fox_ne_sprite.encode());
        rom.set_long(0x120DBC, sprite_ne_addr);
        rom.set_long(0x120DC0, sprite_ne_addr);

        edit_sprite(magic_fox_sw_sprite);
        uint32_t sprite_sw_addr = rom.inject_bytes(magic_fox_sw_sprite.encode());
        rom.set_long(0x120DC4, sprite_sw_addr);
        rom.set_long(0x120DC8, sprite_sw_addr);

        world.entity_type(ENTITY_NPC_MAGIC_FOX)->clear_high_palette();
        EntityLowPalette fox_palette = world.entity_type(ENTITY_NPC_MAGIC_FOX)->low_palette();
        fox_palette[0x05] = Color(0x40, 0x20, 0x40); // Put the dark purple that was moved to an unused slot in the actual slot
        world.entity_type(ENTITY_NPC_MAGIC_FOX)->low_palette(fox_palette);

        // Replace the "Duke in chair" sprite by the high palette fox
        auto make_low_palette_sprite_high = [](Sprite& sprite) {
            for(uint8_t i=0x2 ; i<0x8 ; ++i)
                sprite.replace_color(i, i+6);
        };

        Sprite magic_fox_ne_high_sprite = magic_fox_ne_sprite;
        make_low_palette_sprite_high(magic_fox_ne_high_sprite);
        uint32_t sprite_ne_high_addr = rom.inject_bytes(magic_fox_ne_high_sprite.encode());
        rom.set_long(0x120FCC, sprite_ne_high_addr);
        rom.set_long(0x120FD0, sprite_ne_high_addr);

        Sprite magic_fox_sw_high_sprite = magic_fox_sw_sprite;
        make_low_palette_sprite_high(magic_fox_sw_high_sprite);
        uint32_t sprite_sw_high_addr = rom.inject_bytes(magic_fox_sw_high_sprite.encode());
        rom.set_long(0x120FD4, sprite_sw_high_addr);
        rom.set_long(0x120FD8, sprite_sw_high_addr);

        EntityHighPalette fox_high_palette;
        std::copy(fox_palette.begin(), fox_palette.end(), fox_high_palette.begin());
        fox_high_palette[6] = Color(0, 0, 0);
        world.entity_type(ENTITY_NPC_MAGIC_FOX_HIGH_PALETTE)->clear_low_palette();
        world.entity_type(ENTITY_NPC_MAGIC_FOX_HIGH_PALETTE)->high_palette(fox_high_palette);
    }

    static Entity* add_magic_fox(World& world, HintSource* hint_source, uint16_t map_id)
    {
        uint8_t entity_type_id = hint_source->high_palette() ? ENTITY_NPC_MAGIC_FOX_HIGH_PALETTE : ENTITY_NPC_MAGIC_FOX;
        uint8_t palette_id = hint_source->high_palette() ? 1 : 3;

        Map* map = world.map(map_id);
        map->convert_global_masks_into_individual();

        Entity* fox = new Entity({
            .type_id = entity_type_id,
            .position = hint_source->position(),
            .orientation = hint_source->orientation(),
            .palette = palette_id
        });

        map->add_entity(fox);
        return fox;
    }
};
