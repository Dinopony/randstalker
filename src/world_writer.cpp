#include "world_writer.hpp"

#include "model/entity_type.hpp"
#include "model/item.hpp"
#include "model/item_source.hpp"
#include "model/map.hpp"
#include "model/world_teleport_tree.hpp"
#include "model/world_region.hpp"

#include "tools/textbanks_encoder.hpp"
#include "exceptions.hpp"
#include "offsets.hpp"
#include "world.hpp"

void WorldWriter::write_world_to_rom(md::ROM& rom, const World& world)
{
    write_items(rom, world);
    write_item_sources(rom, world);
    write_maps(rom, world);
    write_game_strings(rom, world);
    write_dark_rooms(rom, world);
    write_tibor_tree_connections(rom, world);
    write_fahl_enemies(rom, world);
}


void WorldWriter::write_items(md::ROM& rom, const World& world)
{
    // Prepare a data block for gold values
    uint8_t highest_item_id = world.items().rbegin()->first;
    uint8_t gold_items_count = (highest_item_id - ITEM_GOLDS_START) + 1;
    uint32_t gold_values_table_addr = rom.reserve_data_block(gold_items_count, "data_gold_values");

    // Write actual items
    for(auto& [item_id, item] : world.items())
    {
        // Special treatment for gold items (only write their value in the previously prepared data block)
        if(item->id() >= ITEM_GOLDS_START)
        {
            uint32_t addr = gold_values_table_addr + (item_id - ITEM_GOLDS_START);
            rom.set_byte(addr, static_cast<uint8_t>(world.items().at(item_id)->gold_value()));
            continue;
        }

        uint32_t item_base_addr = offsets::ITEM_DATA_TABLE + item_id * 0x04;

        // Set max quantity
        uint8_t verb_and_max_qty = rom.get_byte(item_base_addr);
        verb_and_max_qty &= 0xF0;
        verb_and_max_qty += item->max_quantity();
        rom.set_byte(item_base_addr, verb_and_max_qty);

        // Set gold value
        rom.set_word(item_base_addr + 0x2, item->gold_value());
    }
}

void WorldWriter::write_item_sources(md::ROM& rom, const World& world)
{
    for(ItemSource* source : world.item_sources())
    {
        if(source->type_name() == "chest")
        {
            uint8_t chest_id = reinterpret_cast<ItemSourceChest*>(source)->chest_id();
            rom.set_byte(0x9EABE + chest_id, source->item_id());
        }
        else if(source->type_name() == "reward")
        {
            uint32_t address_in_rom = reinterpret_cast<ItemSourceReward*>(source)->address_in_rom();
            rom.set_byte(address_in_rom, source->item_id());
        }
        // Ground & shop item sources are tied to map entities that are updated as their contents change.
        // Therefore those types of item sources will effectively be written when map entities are written.
    }
}

void WorldWriter::write_maps(md::ROM& rom, const World& world)
{
    uint16_t cumulated_offset_entities = 0x0;
    uint32_t variants_table_current_addr = offsets::MAP_VARIANTS_TABLE;
    uint32_t entity_masks_table_current_addr = offsets::MAP_ENTITY_MASKS_TABLE;
    uint32_t clear_flags_table_current_addr = offsets::MAP_CLEAR_FLAGS_TABLE;

    std::vector<std::pair<uint16_t, MapVariant>> variants;

    for(auto& [map_id, map] : world.maps())
    {
        map->write_to_rom(rom);
        
        // Write map entities
        if(!map->entities().empty())
        {
            rom.set_word(offsets::MAP_ENTITIES_OFFSETS_TABLE + (map_id*2), cumulated_offset_entities + 1);
            std::vector<uint8_t> entity_bytes = map->entities_as_bytes();

            rom.set_bytes(offsets::MAP_ENTITIES_TABLE + cumulated_offset_entities, entity_bytes);
            cumulated_offset_entities += (uint32_t)entity_bytes.size();
        }
        else
        {
            rom.set_word(offsets::MAP_ENTITIES_OFFSETS_TABLE + (map_id*2), 0x0000);
        }

        // Write map variants
        for(const MapVariant& variant : map->variants())
            variants.push_back(std::make_pair(map_id, variant));

        // Write entity masks
        for(const EntityMask& mask : map->entity_masks())
        {
            uint16_t mask_bytes = mask.to_bytes();
            rom.set_word(entity_masks_table_current_addr, map->id());
            rom.set_word(entity_masks_table_current_addr+2, mask_bytes);
            entity_masks_table_current_addr += 0x4;
        }

        // Write clear flags
        for(const ClearFlag& clear_flag : map->clear_flags())
        {
            uint16_t flag_bytes = clear_flag.to_bytes();
            rom.set_word(clear_flags_table_current_addr, map->id());
            rom.set_word(clear_flags_table_current_addr+2, flag_bytes);
            clear_flags_table_current_addr += 0x4;
        }
    }

    // We process variants reversed so that when several variants are available,
    // the last one in the list is the one coming from the "origin" map. This is especially
    // important since the game only defines map exits for the origin map (not the variants)
    // and the algorithm it uses to find back the origin map takes the last find in the list.
    std::reverse(variants.begin(), variants.end());
    for(auto& [map_id, variant] : variants)
    {
        rom.set_word(variants_table_current_addr, map_id);
        rom.set_word(variants_table_current_addr+2, variant.map_variant_id);
        rom.set_byte(variants_table_current_addr+4, variant.flag_byte);
        rom.set_byte(variants_table_current_addr+5, variant.flag_bit);
        variants_table_current_addr += 0x6;
    }

    if(cumulated_offset_entities > offsets::MAP_ENTITIES_TABLE_END)
        throw RandomizerException("Entities must not be bigger than the one from base game");
    rom.mark_empty_chunk(offsets::MAP_ENTITIES_TABLE + cumulated_offset_entities, offsets::MAP_ENTITIES_TABLE_END);

    rom.set_long(variants_table_current_addr, 0xFFFFFFFF);
    variants_table_current_addr += 0x4;
    if(variants_table_current_addr > offsets::MAP_VARIANTS_TABLE_END)
        throw RandomizerException("Map variants must not be bigger than the one from base game");
    rom.mark_empty_chunk(variants_table_current_addr, offsets::MAP_VARIANTS_TABLE_END);
    
    rom.set_word(entity_masks_table_current_addr, 0xFFFF);
    entity_masks_table_current_addr += 0x2;
    if(entity_masks_table_current_addr > offsets::MAP_ENTITY_MASKS_TABLE_END)
        throw RandomizerException("Map entity masks table must not be bigger than the one from base game");
    rom.mark_empty_chunk(entity_masks_table_current_addr, offsets::MAP_ENTITY_MASKS_TABLE_END);

    rom.set_word(clear_flags_table_current_addr, 0xFFFF);
    clear_flags_table_current_addr += 0x2;
    if(clear_flags_table_current_addr > offsets::MAP_CLEAR_FLAGS_TABLE_END)
        throw RandomizerException("Map clear flags table must not be bigger than the one from base game");
    rom.mark_empty_chunk(clear_flags_table_current_addr, offsets::MAP_CLEAR_FLAGS_TABLE_END);
}

void WorldWriter::write_game_strings(md::ROM& rom, const World& world)
{
    TextbanksEncoder encoder(rom, world.game_strings());
    encoder.write_to_rom(rom);
}

void WorldWriter::write_dark_rooms(md::ROM& rom, const World& world)
{
    // Inject dark rooms as a data block
    const std::vector<uint16_t>& dark_map_ids = world.dark_region()->dark_map_ids();
    uint16_t dark_maps_byte_count = static_cast<uint16_t>(dark_map_ids.size() + 1) * 0x02;
    uint32_t dark_maps_address = rom.reserve_data_block(dark_maps_byte_count, "data_dark_rooms");
    uint8_t i = 0;
    for (uint16_t map_id : dark_map_ids)
        rom.set_word(dark_maps_address + (i++) * 0x2, map_id);
    rom.set_word(dark_maps_address + i * 0x2, 0xFFFF);
}

void WorldWriter::write_tibor_tree_connections(md::ROM& rom, const World& world)
{
    for (auto& [tree_1, tree_2] : world.teleport_tree_pairs())
    {
        tree_1->write_to_rom(rom);
        tree_2->write_to_rom(rom);
    }
}

void WorldWriter::write_fahl_enemies(md::ROM& rom, const World& world)
{
    if(world.fahl_enemies().size() > 50)
        throw RandomizerException("Cannot put more than 50 enemies for Fahl challenge");

    for(uint8_t i=0 ; i < world.fahl_enemies().size() ; ++i)
        rom.set_byte(0x12CE6 + i, world.fahl_enemies().at(i)->id());
}
