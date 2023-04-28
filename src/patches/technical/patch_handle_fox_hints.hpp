#pragma once

#include <landstalker-lib/patches/game_patch.hpp>

#include <landstalker-lib/model/entity.hpp>
#include <landstalker-lib/model/entity_type.hpp>
#include <landstalker-lib/model/map.hpp>
#include <landstalker-lib/model/world.hpp>
#include <landstalker-lib/constants/entity_type_codes.hpp>
#include <landstalker-lib/tools/byte_array.hpp>
#include <landstalker-lib/tools/vectools.hpp>
#include <landstalker-lib/constants/symbols.hpp>
#include <landstalker-lib/tools/sprite.hpp>
#include "../../logic_model/randomizer_world.hpp"
#include "../../logic_model/hint_source.hpp"

class PatchHandleFoxHints : public GamePatch
{
private:
    ByteArray _hint_map_ids_table;
    ByteArray _command_words_table;
    uint32_t _hint_map_ids_addr = 0xFFFFFFFF;
    uint32_t _hints_dialogue_commands_block = 0xFFFFFFFF;

    static constexpr uint32_t KAYLA_DIALOGUE_SCRIPT = 0x27222;
    static constexpr uint32_t KAYLA_DIALOGUE_SCRIPT_END = 0x27260;
    static constexpr uint8_t ENTITY_NPC_MAGIC_FOX_HIGH_PALETTE = ENTITY_NPC_DUKE_CHAIR;
    static constexpr uint8_t MAGIC_FOX_SPEAKER_ID = 0x2E; // Kayla's speaker ID

public:
    void alter_rom(md::ROM& rom) override
    {
        // Set foxes voice to the same one as Mir (replacing now unused "Kayla" and "Kayla in bath" entries)
        rom.set_byte(0x2910A, ENTITY_NPC_MAGIC_FOX);
        rom.set_byte(0x2910B, 0x6A);
        rom.set_byte(0x2910C, ENTITY_NPC_MAGIC_FOX_HIGH_PALETTE);
        rom.set_byte(0x2910D, 0x6A);

        // Set their name to "Foxy"
        std::string name = "Foxy";
        rom.set_byte(0x2968A, name.length());
        rom.set_bytes(0x2968B, Symbols::bytes_for_symbols(name));

        // Compensate for the size change in Kayla's name string by changing Wally's name string
        rom.set_byte(0x2968B + name.length(), 0x05 + (0x5 - name.length()));
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
                // Add Magic Foxes speaker ID (former Kayla ID) to the list of speakers in parent map
                Map* parent_map = world.map(map_id);
                while(parent_map->parent_map())
                    parent_map = parent_map->parent_map();

                if(!vectools::contains(processed_map_ids, parent_map->id()))
                {
                    // Add the required data for the fox to actually talk
                    parent_map->speaker_ids().emplace_back(MAGIC_FOX_SPEAKER_ID);
                    _hint_map_ids_table.add_word(parent_map->id());
                    _command_words_table.add_word(0xE000 | ((hint_source->text_ids()[0] - 0x4D) & 0x1FFF));

                    processed_map_ids.emplace_back(parent_map->id());
                }

                uint8_t dialogue_id = parent_map->speaker_ids().size() - 1;

                // Add the actual fox inside the map where it is meant to be
                add_magic_fox(world, hint_source, map_id, dialogue_id);
            }
        }
    }

    void inject_data(md::ROM& rom, World& world) override
    {
        inject_altered_fox_sprites(rom, world);

        // Inject the LUT of map IDs to match with hints IDs
        _hint_map_ids_addr = rom.inject_bytes(_hint_map_ids_table);

        // Inject the actual dialogue command scripts
        _hints_dialogue_commands_block = rom.inject_bytes(_command_words_table);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        // Inject the function capable of using that LUT, and call it on foxes universal script (former Kayla's script)
        uint32_t func_handle_hints = inject_func_handle_hints(rom);
        rom.set_code(KAYLA_DIALOGUE_SCRIPT, md::Code().jmp(func_handle_hints));

        // Set a height of 2 tiles for Foxies (like any other NPC in the game) instead of 2.5
        rom.set_byte(0x1B05D, 0x20);
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

    uint32_t inject_func_handle_hints(md::ROM& rom) const
    {
        md::Code func_handle_hints;
        func_handle_hints.movem_to_stack({ reg_D0, reg_D1, reg_D2 }, { reg_A0 });
        {
            func_handle_hints.movew(addr_(0xFF1206), reg_D2);
            func_handle_hints.movew(0x0001, reg_D0);
            func_handle_hints.lea(_hint_map_ids_addr, reg_A0);
            func_handle_hints.label("loop_start");
            {
                func_handle_hints.movew(addr_(reg_A0), reg_D1);
                func_handle_hints.bmi("return"); // Reached end of list without finding the appropriate map
                func_handle_hints.cmpw(reg_D1, reg_D2);
                func_handle_hints.beq("map_found");
                // Not the right map, point on next value and loop back
                func_handle_hints.adda(0x2, reg_A0);
                func_handle_hints.addqw(0x2, reg_D0);
            }
            func_handle_hints.bra("loop_start");
            func_handle_hints.label("map_found");
            {
                // Right map found
                func_handle_hints.lea(_hints_dialogue_commands_block, reg_A0);
                func_handle_hints.jsr(0x253F8); // RunTextCmd function
            }
            func_handle_hints.label("return");
        }
        func_handle_hints.movem_from_stack({ reg_D0, reg_D1, reg_D2 }, { reg_A0 });
        func_handle_hints.rts();

        return rom.inject_code(func_handle_hints);
    }

    static void add_magic_fox(World& world, HintSource* hint_source, uint16_t map_id, uint8_t dialogue_id)
    {
        uint8_t entity_type_id = hint_source->high_palette() ? ENTITY_NPC_MAGIC_FOX_HIGH_PALETTE : ENTITY_NPC_MAGIC_FOX;
        uint8_t palette_id = hint_source->high_palette() ? 1 : 3;

        Map* map = world.map(map_id);
        map->convert_global_masks_into_individual();

        map->add_entity(new Entity({
            .type_id = entity_type_id,
            .position = hint_source->position(),
            .orientation = hint_source->orientation(),
            .palette = palette_id,
            .talkable = true,
            .dialogue = dialogue_id,
        }));
    }
};
