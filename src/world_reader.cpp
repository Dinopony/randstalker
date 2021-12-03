#include "world_reader.hpp"

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

void WorldReader::read_maps(World& world, const md::ROM& rom)
{
    read_maps_data(world, rom);
    read_maps_fall_destination(world, rom);
    read_maps_climb_destination(world, rom);
    read_maps_entities(world, rom);
    read_maps_exits(world, rom);
    read_maps_variants(world, rom);
    read_maps_entity_masks(world, rom);
    read_maps_global_entity_masks(world, rom);
    read_maps_dialogue_table(world, rom);
}

void WorldReader::read_maps_data(World& world, const md::ROM& rom)
{
    constexpr uint16_t MAP_COUNT = 816;
    for(uint16_t map_id = 0 ; map_id < MAP_COUNT ; ++map_id)
    {
        Map* map = new Map(map_id);

        uint32_t addr = offsets::MAP_DATA_TABLE + (map_id * 8);

        map->address(rom.get_long(addr));

        map->tileset_id(rom.get_byte(addr+4) & 0x1F);
        map->primary_big_tileset_id((rom.get_byte(addr+4) >> 5) & 0x01);
        map->unknown_param_1((rom.get_byte(addr+4) >> 6));

        map->palette_id(rom.get_byte(addr+5) & 0x3F);
        map->unknown_param_2((rom.get_byte(addr+5) >> 6));

        map->room_height(rom.get_byte(addr+6));

        map->background_music(rom.get_byte(addr+7) & 0x1F);
        map->secondary_big_tileset_id((rom.get_byte(addr+7) >> 5) & 0x07);
        
        // Read base chest ID from its dedicated table
        map->base_chest_id(rom.get_byte(offsets::MAP_BASE_CHEST_ID_TABLE + map_id));

        // Read visited flag from its dedicated table
        uint16_t flag_description = rom.get_word(offsets::MAP_VISITED_FLAG_TABLE + (map_id * 2));
        uint16_t byte = (flag_description >> 3) + 0xC0;
        uint8_t bit = flag_description & 0x7;
        map->visited_flag(Flag(byte, bit));

        world.set_map(map_id, map);
    }
}

void WorldReader::read_maps_fall_destination(World& world, const md::ROM& rom)
{
    for(uint32_t addr = offsets::MAP_FALL_DESTINATION_TABLE ; rom.get_word(addr) != 0xFFFF ; addr += 0x4)
    {
        Map* map = world.map(rom.get_word(addr));
        map->fall_destination(rom.get_word(addr+2));
    }
}

void WorldReader::read_maps_climb_destination(World& world, const md::ROM& rom)
{
    for(uint32_t addr = offsets::MAP_CLIMB_DESTINATION_TABLE ; rom.get_word(addr) != 0xFFFF ; addr += 0x4)
    {
        Map* map = world.map(rom.get_word(addr));
        map->climb_destination(rom.get_word(addr+2));
    }
}

void WorldReader::read_maps_entities(World& world, const md::ROM& rom)
{
    for(auto& [map_id, map] : world.maps())
    {
        uint16_t offset = rom.get_word(offsets::MAP_ENTITIES_OFFSETS_TABLE + (map_id*2));
        if(offset > 0)
        {
            // Maps with offset 0000 have no entities
            for(uint32_t addr = offsets::MAP_ENTITIES_TABLE + offset-1 ; rom.get_word(addr) != 0xFFFF ; addr += 0x8)
                map->add_entity(Entity::from_rom(rom, addr, map));
        }
    }
}

void WorldReader::read_maps_exits(World& world, const md::ROM& rom)
{
    for(uint32_t addr = offsets::MAP_EXITS_TABLE ; rom.get_word(addr) != 0xFFFF ; addr += 0x8)
    {
        uint16_t map_id_1 = rom.get_word(addr) & 0x3FF;
        uint8_t extra_1 = (rom.get_byte(addr) & 0xFC) >> 2;
        uint8_t pos_x_1 = rom.get_byte(addr+2);
        uint8_t pos_y_1 = rom.get_byte(addr+3);

        uint16_t map_id_2 = rom.get_word(addr+4) & 0x3FF;
        uint8_t extra_2 = (rom.get_byte(addr+4) & 0xFC) >> 2;
        uint8_t pos_x_2 = rom.get_byte(addr+6);
        uint8_t pos_y_2 = rom.get_byte(addr+7);

        MapExit exit;
        exit.pos_x = pos_x_1;
        exit.pos_y = pos_y_1;
        exit.extra_byte = extra_1;
        exit.destination_map_id = map_id_2;
        exit.destination_x = pos_x_2;
        exit.destination_y = pos_y_2;
        world.map(map_id_1)->exits().push_back(exit);

        MapExit exit_reverse;
        exit_reverse.pos_x = pos_x_2;
        exit_reverse.pos_y = pos_y_2;
        exit_reverse.extra_byte = extra_2;
        exit_reverse.destination_map_id = map_id_1;
        exit_reverse.destination_x = pos_x_1;
        exit_reverse.destination_y = pos_y_1;
        world.map(map_id_2)->exits().push_back(exit_reverse);
    }
}

void WorldReader::read_maps_variants(World& world, const md::ROM& rom)
{
    for(uint32_t addr = offsets::MAP_VARIANTS_TABLE ; rom.get_word(addr) != 0xFFFF ; addr += 0x6)
    {
        Map* map = world.map(rom.get_word(addr));
        Map* variant_map = world.map(rom.get_word(addr+2));

        uint8_t flag_byte = rom.get_byte(addr+4);
        uint8_t flag_bit = rom.get_byte(addr+5);
        
        map->add_variant(variant_map, flag_byte, flag_bit);
    }
}

void WorldReader::read_maps_entity_masks(World& world, const md::ROM& rom)
{
    for(uint32_t addr = offsets::MAP_ENTITY_MASKS_TABLE ; rom.get_word(addr) != 0xFFFF ; addr += 0x4)
    {
        Map* map = world.map(rom.get_word(addr));

        uint16_t word = rom.get_word(addr+2);

        uint8_t msb = word >> 8;
        uint8_t lsb = word & 0x00FF;

        bool visibility_if_flag_set = msb >> 7;
        uint8_t flag_byte = msb & 0x7F;
        uint8_t flag_bit = lsb >> 5;
        uint8_t entity_id = lsb & 0x0F;

        map->entities().at(entity_id)->mask_flags().push_back(EntityMaskFlag(visibility_if_flag_set, flag_byte, flag_bit));
    }
}

void WorldReader::read_maps_global_entity_masks(World& world, const md::ROM& rom)
{
    for(uint32_t addr = offsets::MAP_CLEAR_FLAGS_TABLE ; rom.get_word(addr) != 0xFFFF ; addr += 0x4)
    {
        Map* map = world.map(rom.get_word(addr));

        uint8_t flag_byte = rom.get_byte(addr+2);

        uint8_t lsb = rom.get_byte(addr+3);
        uint8_t flag_bit = lsb >> 5;
        uint8_t first_entity_id = lsb & 0x1F;
 
        map->global_entity_mask_flags().push_back(GlobalEntityMaskFlag(flag_byte, flag_bit, first_entity_id));
    }
}

void WorldReader::read_maps_dialogue_table(World& world, const md::ROM& rom)
{
    uint32_t addr = offsets::DIALOGUE_TABLE;

    uint16_t header_word = rom.get_word(addr);
    while(header_word != 0xFFFF)
    {
        uint16_t map_id = header_word & 0x7FF;
        uint8_t word_count = header_word >> 11;

        Map* map = world.map(map_id);
        std::vector<uint16_t>& map_speakers = map->speaker_ids();

        for(uint8_t i=0 ; i<word_count ; ++i)
        {
            uint32_t offset = (i+1)*2;
            uint16_t word = rom.get_word(addr + offset);
            
            uint16_t speaker_id = word & 0x7FF;
            uint8_t consecutive_speakers = word >> 11;
            for(uint8_t j=0 ; j<consecutive_speakers ; ++j)
                map_speakers.push_back(speaker_id++);
        }

        addr += (word_count + 1) * 2;
        header_word = rom.get_word(addr);
    }
}
