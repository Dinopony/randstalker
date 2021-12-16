#include <landstalker_lib/md_tools.hpp>
#include <landstalker_lib/model/entity.hpp>
#include <landstalker_lib/model/entity_type.hpp>
#include <landstalker_lib/model/map.hpp>
#include <landstalker_lib/model/world.hpp>
#include <landstalker_lib/constants/entity_type_codes.hpp>
#include "landstalker_lib/tools/byte_array.hpp"
#include <landstalker_lib/tools/huffman/symbols.hpp>
#include <landstalker_lib/tools/sprite.hpp>
#include "../logic_model/randomizer_world.hpp"
#include "../logic_model/hint_source.hpp"
#include "../world_shuffler.hpp"

constexpr uint32_t KAYLA_DIALOGUE_SCRIPT = 0x27222;
constexpr uint32_t KAYLA_DIALOGUE_SCRIPT_END = 0x27260;

static uint32_t inject_func_handle_hints(md::ROM& rom, uint32_t hint_map_ids_addr, uint32_t hints_dialogue_commands_block)
{
    md::Code func_handle_hints;
    func_handle_hints.movem_to_stack({reg_D0, reg_D1, reg_D2}, { reg_A0 });
    func_handle_hints.movew(addr_(0xFF1206), reg_D2);
    func_handle_hints.movew(0x0001, reg_D0);
    func_handle_hints.lea(hint_map_ids_addr, reg_A0);
    func_handle_hints.label("loop_start");
    func_handle_hints.movew(addr_(reg_A0), reg_D1);
    func_handle_hints.bpl(2);
        // Reached end of list without finding the appropriate map
        func_handle_hints.bra(8);
    func_handle_hints.cmpw(reg_D1, reg_D2);
    func_handle_hints.beq(4);
        // Not the right map, point on next value and loop back
        func_handle_hints.adda(0x2, reg_A0);
        func_handle_hints.addqw(0x2, reg_D0);
        func_handle_hints.bra("loop_start");
    // Right map found
    func_handle_hints.lea(hints_dialogue_commands_block, reg_A0);
    func_handle_hints.jsr(0x253F8); // RunTextCmd function
    func_handle_hints.movem_from_stack({reg_D0, reg_D1, reg_D2}, { reg_A0 });
    func_handle_hints.rts();

    return rom.inject_code(func_handle_hints);
}

static uint16_t add_magic_fox(World& world, HintSource* hint_source_entity)
{
    Map* map = world.map(hint_source_entity->map_ids()[0]);

    // Add Magic Foxes speaker ID to the map speakers (former Kayla ID)
    Map* parent_map = map;
    while(parent_map->parent_map())
        parent_map = parent_map->parent_map();

    parent_map->speaker_ids().emplace_back(0x2E);
    uint8_t dialogue_id = parent_map->speaker_ids().size() - 1;

    for(uint16_t map_id : hint_source_entity->map_ids())
    {
        world.map(map_id)->add_entity(new Entity({
            .type_id = ENTITY_NPC_MAGIC_FOX,
            .position = hint_source_entity->position(),
            .orientation = hint_source_entity->orientation(),
            .palette = 3,
            .talkable = true,
            .dialogue = dialogue_id,
        }));
    }

    return parent_map->id();
}

static void edit_fox_voice_and_name(md::ROM& rom)
{
    // Set foxes voice to the same one as Mir (replacing now unused "Kayla in bath" entry)
    rom.set_byte(0x2910C, ENTITY_NPC_MAGIC_FOX);
    rom.set_byte(0x2910D, 0x6A);

    // Set their name to "Foxy"
    std::string name = "Foxy";
    rom.set_byte(0x2968A, name.length());
    rom.set_bytes(0x2968B, Symbols::bytes_for_symbols(name));

    // Compensate for the size change in Kayla's name string by changing Wally's name string
    rom.set_byte(0x2968B + name.length(), 0x05 + (0x5 - name.length()));
}

static void make_magic_fox_only_use_low_palette(md::ROM& rom, World& world)
{
    // Anim 0 Frame 0 = NE ---> SpriteGfx006Frame00 ---> 180D56
    // Anim 1 Frame 0 = SW ---> SpriteGfx006Frame01 ---> 181038
    //                                           Next is 18131C
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
    EntityLowPaletteColors fox_palette = world.entity_type(ENTITY_NPC_MAGIC_FOX)->low_palette();
    fox_palette[0x05] = 0x424; // Put the dark purple that was moved to an unused slot in the actual slot
    world.entity_type(ENTITY_NPC_MAGIC_FOX)->low_palette(fox_palette);
}

void handle_fox_hints(md::ROM& rom, RandomizerWorld& world)
{
    ByteArray hint_map_ids_table;
    ByteArray command_words_table;
    for(HintSource* source : world.used_hint_sources())
    {
        // If hint source is special, it means it's not a fox
        if(source->special())
            continue;

        // Add the actual fox inside the map where it is meant to be
        uint16_t parent_map_id = add_magic_fox(world, source);
        hint_map_ids_table.add_word(parent_map_id);

        // Add the command words table triggering one fox dialogue each
        command_words_table.add_word(0xE000 | ((source->text_ids()[0] - 0x4D) & 0x1FFF));
    }

    // Inject the LUT of map IDs to match with hints IDs
    uint32_t hint_map_ids_addr = rom.inject_bytes(hint_map_ids_table);
    // Inject the actual dialogue command scripts
    uint32_t hints_dialogue_commands_block = rom.inject_bytes(command_words_table);

    // Inject the function capable of using that LUT, and call it on foxes universal script (former Kayla's script)
    uint32_t func_handle_hints = inject_func_handle_hints(rom, hint_map_ids_addr, hints_dialogue_commands_block);
    rom.set_code(KAYLA_DIALOGUE_SCRIPT, md::Code().jmp(func_handle_hints));

    // Edit the voice used when foxes talk, as well as their name
    edit_fox_voice_and_name(rom);
    make_magic_fox_only_use_low_palette(rom, world);
}
