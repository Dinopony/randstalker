#include "world_writer.hpp"

#include "world_model/entity_type.hpp"
#include "world_model/item.hpp"
#include "world_model/item_source.hpp"
#include "world_model/map.hpp"
#include "world_model/map_palette.hpp"
#include "world_model/world_teleport_tree.hpp"
#include "world_model/world.hpp"

#include "tools/byte_array.hpp"
#include "tools/textbanks_encoder.hpp"
#include "exceptions.hpp"

#include "constants/offsets.hpp"
#include "constants/entity_type_codes.hpp"

void WorldWriter::write_world_to_rom(md::ROM& rom, const World& world)
{
    write_items(rom, world);
    write_item_sources(rom, world);
    write_entity_types(rom, world);
    write_game_strings(rom, world);
    write_dark_rooms(rom, world);
    write_tibor_tree_connections(rom, world);
    write_fahl_enemies(rom, world);
    write_map_connections(rom, world);
    write_map_palettes(rom, world);
    write_maps(rom, world);
}

///////////////////////////////////////////////////////////////////////////////

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
        else
        {
            // Ground & shop item sources are tied to map entities that are updated as their contents change.
            // Therefore those types of item sources will effectively be written when map entities are written.
            ItemSourceOnGround* ground_source = reinterpret_cast<ItemSourceOnGround*>(source);
            for (Entity* entity : ground_source->entities())
                entity->entity_type_id(ground_source->item_id() + 0xC0);
        }
    }
}

void WorldWriter::write_entity_types(md::ROM& rom, const World& world)
{
    std::vector<EntityEnemy*> enemy_types;
    std::set<uint16_t> drop_probabilities;
    std::map<uint16_t, uint8_t> drop_probability_lookup;
    std::map<EntityEnemy*, uint16_t> instances_in_game;

    // List all different enemy types and different drop probabilities
    // Also count the number of instances for each enemy in the game to optimize
    // stats loading in game.
    for(auto& [id, entity_type] : world.entity_types())
    {
        if(entity_type->type_name() != "enemy")
            continue;
        EntityEnemy* enemy_type = reinterpret_cast<EntityEnemy*>(entity_type);

        enemy_types.push_back(enemy_type);
        drop_probabilities.insert(enemy_type->drop_probability());

        uint32_t count = 0;
        for(auto& [map_id, map] : world.maps())
        {
            for(Entity* entity : map->entities())
            {
                if(entity->entity_type_id() == entity_type->id())
                    count++;
            }
        }
        instances_in_game[enemy_type] = count;
    }

    // Sort the enemy types by number of instances in game (descending)
    std::sort(enemy_types.begin(), enemy_types.end(), 
        [instances_in_game](EntityEnemy* a, EntityEnemy* b) -> bool {
            return instances_in_game.at(a) > instances_in_game.at(b);
        }
    );

    // Build a lookup table with probabilities
    if(drop_probabilities.size() > 8)
        throw RandomizerException("Having more than 8 different loot probabilities is not allowed");

    uint8_t current_id = 0;
    uint32_t addr = offsets::PROBABILITY_TABLE;
    for(uint16_t probability : drop_probabilities)
    {
        drop_probability_lookup[probability] = current_id++;
        rom.set_word(addr, probability);
        addr += 0x2;
    }

    // Write the actual enemy stats
    addr = offsets::ENEMY_STATS_TABLE;
    for(EntityEnemy* enemy_type : enemy_types)
    {
        uint8_t byte5 = enemy_type->attack() & 0x7F;
        uint8_t byte6 = enemy_type->dropped_item_id() & 0x3F;

        uint8_t probability_id = drop_probability_lookup.at(enemy_type->drop_probability());
        byte5 |= (probability_id & 0x4) << 5;
        byte6 |= (probability_id & 0x3) << 6;

        rom.set_byte(addr, enemy_type->id());
        rom.set_byte(addr+1, enemy_type->health());
        rom.set_byte(addr+2, enemy_type->defence());
        rom.set_byte(addr+3, enemy_type->dropped_golds());
        rom.set_byte(addr+4, byte5);
        rom.set_byte(addr+5, byte6);
        addr += 0x6;
    }
    rom.set_word(addr, 0xFFFF);
    addr += 0x2;
    if(addr > offsets::ENEMY_STATS_TABLE_END)
        throw RandomizerException("Enemy stats table is bigger than in original game");
}

void WorldWriter::write_game_strings(md::ROM& rom, const World& world)
{
    TextbanksEncoder encoder(rom, world.game_strings());
    encoder.write_to_rom(rom);
}

void WorldWriter::write_dark_rooms(md::ROM& rom, const World& world)
{
    // Inject dark rooms as a data block
    ByteArray bytes;
    for (uint16_t map_id : world.dark_maps())
        bytes.add_word(map_id);
    bytes.add_word(0xFFFF);

    rom.inject_bytes(bytes, "data_dark_rooms");
}

void WorldWriter::write_tibor_tree_connections(md::ROM& rom, const World& world)
{
    for (auto& [tree_1, tree_2] : world.teleport_tree_pairs())
    {
        rom.set_word(tree_1->left_entrance_address(), tree_1->tree_map_id());
        rom.set_word(tree_1->right_entrance_address(), tree_1->tree_map_id());
        rom.set_word(tree_2->left_entrance_address(), tree_2->tree_map_id());
        rom.set_word(tree_2->right_entrance_address(), tree_2->tree_map_id());
    }
}

void WorldWriter::write_fahl_enemies(md::ROM& rom, const World& world)
{
    if(world.fahl_enemies().size() > 50)
        throw RandomizerException("Cannot put more than 50 enemies for Fahl challenge");

    for(uint8_t i=0 ; i < world.fahl_enemies().size() ; ++i)
        rom.set_byte(0x12CE6 + i, world.fahl_enemies().at(i)->id());
}


void WorldWriter::write_map_connections(md::ROM& rom, const World& world)
{
    uint32_t addr = offsets::MAP_CONNECTIONS_TABLE;

    for(const MapConnection& connection : world.map_connections())
    {
        uint8_t byte_1 = (connection.map_id_1() >> 8) & 0x03;
        byte_1 |= (connection.extra_byte_1() << 2) & 0xFC;
        uint8_t byte_2 = connection.map_id_1() & 0xFF;
        uint8_t byte_3 = connection.pos_x_1();
        uint8_t byte_4 = connection.pos_y_1();

        uint8_t byte_5 = (connection.map_id_2() >> 8) & 0x03;
        byte_5 |= (connection.extra_byte_2() << 2) & 0xFC;
        uint8_t byte_6 = connection.map_id_2() & 0xFF;
        uint8_t byte_7 = connection.pos_x_2();
        uint8_t byte_8 = connection.pos_y_2();

        rom.set_bytes(addr, { byte_1, byte_2, byte_3, byte_4, byte_5, byte_6, byte_7, byte_8 });
        addr += 0x8;
    }
    
    rom.set_word(addr, 0xFFFF);
    addr += 0x2;
    if(addr > offsets::MAP_CONNECTIONS_TABLE_END)
        throw RandomizerException("Map connections table is bigger than in original game");
    rom.mark_empty_chunk(addr, offsets::MAP_CONNECTIONS_TABLE_END);
}


void WorldWriter::write_map_palettes(md::ROM& rom, const World& world)
{
    ByteArray palette_table_bytes;

    for(const MapPalette* palette : world.map_palettes())
    {
        for(const MapPalette::Color& color : palette->colors())
            palette_table_bytes.add_word(color.to_word());
    }

    rom.mark_empty_chunk(offsets::MAP_PALETTES_TABLE, offsets::MAP_PALETTES_TABLE_END);
    uint32_t new_palette_table_addr = rom.inject_bytes(palette_table_bytes);
    rom.set_long(offsets::MAP_PALETTES_TABLE_POINTER, new_palette_table_addr);
}

///////////////////////////////////////////////////////////////////////////////

void WorldWriter::write_maps(md::ROM& rom, const World& world)
{
    write_maps_data(rom, world);
    write_maps_climb_destination(rom, world);
    write_maps_fall_destination(rom, world);
    write_maps_variants(rom, world);
    write_maps_global_entity_masks(rom, world);
    write_maps_entity_masks(rom, world);
    write_maps_dialogue_table(rom, world);
    write_maps_entities(rom, world);
    write_maps_entity_persistence_flags(rom, world);
}

void WorldWriter::write_maps_data(md::ROM& rom, const World& world)
{
    for(auto& [map_id, map] : world.maps())
    {
        uint32_t addr = offsets::MAP_DATA_TABLE + (map_id * 8);

        uint8_t byte4;
        byte4 = map->tileset_id() & 0x1F;
        byte4 |= (map->primary_big_tileset_id() & 0x01) << 5;
        byte4 |= (map->unknown_param_1() & 0x03) << 6;
        
        uint8_t byte5;
        byte5 = world.map_palette_id(map->palette()) & 0x3F;
        byte5 |= (map->unknown_param_2() & 0x03) << 6;
        
        uint8_t byte7;
        byte7 = map->background_music() & 0x1F;
        byte7 |= (map->secondary_big_tileset_id() & 0x07) << 5;
        
        rom.set_long(addr, map->address());
        rom.set_byte(addr+4, byte4);
        rom.set_byte(addr+5, byte5);
        rom.set_byte(addr+6, map->room_height());
        rom.set_byte(addr+7, byte7);

        // Write map base chest id
        rom.set_byte(offsets::MAP_BASE_CHEST_ID_TABLE + map_id, map->base_chest_id());

        // Write map visited flag
        uint16_t flag_word = (map->visited_flag().byte << 3) + map->visited_flag().bit;
        rom.set_word(offsets::MAP_VISITED_FLAG_TABLE + (map->id()*2), flag_word);
    }
}

void WorldWriter::write_maps_fall_destination(md::ROM& rom, const World& world)
{
    uint32_t addr = offsets::MAP_FALL_DESTINATION_TABLE;

    for(auto& [map_id, map] : world.maps())
    {
        uint16_t fall_destination = map->fall_destination();
        if(fall_destination != 0xFFFF)
        {
            rom.set_word(addr, map_id);
            rom.set_word(addr+2, fall_destination);
            addr += 0x4;
        }
    }

    rom.set_word(addr, 0xFFFF);
    addr += 0x2;
    if(addr > offsets::MAP_FALL_DESTINATION_TABLE_END)
        throw RandomizerException("Fall destination table must not be bigger than the one from base game");
}

void WorldWriter::write_maps_climb_destination(md::ROM& rom, const World& world)
{
    uint32_t addr = offsets::MAP_CLIMB_DESTINATION_TABLE;

    for(auto& [map_id, map] : world.maps())
    {
        uint16_t climb_destination = map->climb_destination();
        if(climb_destination != 0xFFFF)
        {
            rom.set_word(addr, map_id);
            rom.set_word(addr+2, climb_destination);
            addr += 0x4;
        }
    }

    rom.set_word(addr, 0xFFFF);
    addr += 0x2;
    if(addr > offsets::MAP_CLIMB_DESTINATION_TABLE_END)
        throw RandomizerException("Climb destination table must not be bigger than the one from base game");
}

void WorldWriter::write_maps_variants(md::ROM& rom, const World& world)
{
    uint32_t addr = offsets::MAP_VARIANTS_TABLE;

    std::map<std::pair<uint16_t, uint16_t>, Flag> variant_flags;
    for(auto& [map_id, map] : world.maps())
    {
        for(auto& [variant_map, flag] : map->variants())
            variant_flags[std::make_pair(map->id(), variant_map->id())] = flag;
    }

    // We process variants reversed so that when several variants are available,
    // the last one in the list is the one coming from the "origin" map. This is especially
    // important since the game only defines map exits for the origin map (not the variants)
    // and the algorithm it uses to find back the origin map takes the last find in the list.
    for(auto it = variant_flags.rbegin() ; it != variant_flags.rend() ; ++it)
    {
        uint16_t map_id = it->first.first;
        uint16_t variant_map_id = it->first.second;
        const Flag& flag = it->second;

        rom.set_word(addr, map_id);
        rom.set_word(addr+2, variant_map_id);
        rom.set_byte(addr+4, (uint8_t)flag.byte);
        rom.set_byte(addr+5, flag.bit);
        addr += 0x6;
    }

    rom.set_long(addr, 0xFFFFFFFF);
    addr += 0x4;
    if(addr > offsets::MAP_VARIANTS_TABLE_END)
        throw RandomizerException("Map variants must not be bigger than the one from base game");
    rom.mark_empty_chunk(addr, offsets::MAP_VARIANTS_TABLE_END);
}

void WorldWriter::write_maps_global_entity_masks(md::ROM& rom, const World& world)
{
    uint32_t addr = offsets::MAP_CLEAR_FLAGS_TABLE;

    for(auto& [map_id, map] : world.maps())
    {
        for(const GlobalEntityMaskFlag& global_mask_flags : map->global_entity_mask_flags())
        {
            uint16_t flag_bytes = global_mask_flags.to_bytes();
            rom.set_word(addr, map->id());
            rom.set_word(addr+2, flag_bytes);
            addr += 0x4;
        }
    }

    rom.set_word(addr, 0xFFFF);
    addr += 0x2;
    if(addr > offsets::MAP_CLEAR_FLAGS_TABLE_END)
        throw RandomizerException("Map clear flags table must not be bigger than the one from base game");
    rom.mark_empty_chunk(addr, offsets::MAP_CLEAR_FLAGS_TABLE_END);
}

void WorldWriter::write_maps_entity_masks(md::ROM& rom, const World& world)
{
    uint32_t addr = offsets::MAP_ENTITY_MASKS_TABLE;

    for(auto& [map_id, map] : world.maps())
    {
        std::vector<uint8_t> bytes; 

        uint8_t entity_id = 0;
        for(Entity* entity : map->entities())
        {
            for(EntityMaskFlag& mask_flag : entity->mask_flags())
            {
                uint8_t flag_msb = mask_flag.byte & 0x7F;
                if(mask_flag.visibility_if_flag_set)
                    flag_msb |= 0x80;

                uint8_t flag_lsb = entity_id & 0x0F;
                flag_lsb |= (mask_flag.bit & 0x7) << 5;

                rom.set_word(addr, map_id);
                rom.set_byte(addr+2, flag_msb);
                rom.set_byte(addr+3, flag_lsb);
                addr += 0x4;
            }
            entity_id++;
        }
    }

    rom.set_word(addr, 0xFFFF);
    addr += 0x2;
    if(addr > offsets::MAP_ENTITY_MASKS_TABLE_END)
        throw RandomizerException("Map entity masks table must not be bigger than the one from base game");
    rom.mark_empty_chunk(addr, offsets::MAP_ENTITY_MASKS_TABLE_END);
}

void WorldWriter::write_maps_dialogue_table(md::ROM& rom, const World& world)
{
    uint32_t addr = offsets::DIALOGUE_TABLE;

    for(auto& [map_id, map] : world.maps())
    {
        const std::vector<uint16_t>& map_speakers = map->speaker_ids();

        std::vector<std::pair<uint16_t, uint8_t>> consecutive_packs;

        uint16_t previous_speaker_id = 0xFFFE;
        uint8_t current_dialogue_id = -1;
        // Assign "dialogue" sequentially to entities
        for(uint16_t speaker_id : map_speakers)
        {
            if(speaker_id != previous_speaker_id)
            {
                if(speaker_id == previous_speaker_id + 1)
                    consecutive_packs[consecutive_packs.size()-1].second++;
                else
                    consecutive_packs.push_back(std::make_pair(speaker_id, 1));

                previous_speaker_id = speaker_id;
                current_dialogue_id++;
            }
        }

        if(!consecutive_packs.empty())
        {
            // Write map header announcing how many dialogue words follow
            uint16_t header_word = map_id + ((uint16_t)consecutive_packs.size() << 11);
            rom.set_word(addr, header_word);
            addr += 0x2;

            // Write speaker IDs in map
            for(auto& pair : consecutive_packs)
            {
                uint16_t speaker_id = pair.first;
                uint8_t consecutive_speakers = pair.second;

                uint16_t pack_word = (speaker_id & 0x7FF) + (consecutive_speakers << 11);
                rom.set_word(addr, pack_word);
                addr += 0x2;
            }
        }
    }

    rom.set_word(addr, 0xFFFF);
    addr += 0x2;
    if(addr > offsets::DIALOGUE_TABLE_END)
        throw RandomizerException("Dialogue table is bigger than in original game");
    rom.mark_empty_chunk(addr, offsets::DIALOGUE_TABLE_END);
}

void WorldWriter::write_maps_entities(md::ROM& rom, const World& world)
{
    uint16_t cumulated_offset = 0x0;
    for(auto& [map_id, map] : world.maps())
    {
        // Write map entities
        if(!map->entities().empty())
        {
            rom.set_word(offsets::MAP_ENTITIES_OFFSETS_TABLE + (map_id*2), cumulated_offset + 1);

            for(Entity* entity : map->entities())
            {
                std::vector<uint8_t> entity_bytes = entity->to_bytes();
                rom.set_bytes(offsets::MAP_ENTITIES_TABLE + cumulated_offset, entity_bytes);
                cumulated_offset += (uint16_t)entity_bytes.size();
            }

            rom.set_word(offsets::MAP_ENTITIES_TABLE + cumulated_offset, 0xFFFF);
            cumulated_offset += 0x2;
        }
        else
        {
            rom.set_word(offsets::MAP_ENTITIES_OFFSETS_TABLE + (map_id*2), 0x0000);
        }
    }

    if(offsets::MAP_ENTITIES_TABLE + cumulated_offset > offsets::MAP_ENTITIES_TABLE_END)
        throw RandomizerException("Entities table must not be bigger than the one from base game");
    rom.mark_empty_chunk(offsets::MAP_ENTITIES_TABLE + cumulated_offset, offsets::MAP_ENTITIES_TABLE_END);
}

void WorldWriter::write_maps_entity_persistence_flags(md::ROM& rom, const World& world)
{
    uint32_t addr = offsets::PERSISTENCE_FLAGS_TABLE;
    std::vector<Entity*> sacred_trees_with_persistence;

    // Write switches persistence flags table
    for(auto& [map_id, map] : world.maps())
    {
        for(Entity* entity : map->entities())
        {
            if(entity->has_persistence_flag())
            {
                if(entity->entity_type_id() != ENTITY_SACRED_TREE)
                {
                    uint8_t flag_byte = entity->persistence_flag().byte & 0xFF;
                    uint8_t flag_bit = entity->persistence_flag().bit;
                    uint8_t entity_id = entity->entity_id();

                    rom.set_word(addr, entity->map()->id());
                    addr += 0x2;
                    rom.set_byte(addr, flag_byte);
                    addr += 0x1;
                    rom.set_byte(addr, (flag_bit << 5) | (entity_id & 0x1F));
                    addr += 0x1;
                }
                else 
                {
                    sacred_trees_with_persistence.push_back(entity);
                }
            }
        }
    }

    rom.set_word(addr, 0xFFFF);
    addr += 0x2;
    if(addr > offsets::SACRED_TREES_PERSISTENCE_FLAGS_TABLE)
        throw RandomizerException("Persistence flags table must not be bigger than the one from base game");
    rom.mark_empty_chunk(addr, offsets::SACRED_TREES_PERSISTENCE_FLAGS_TABLE);
    
    addr = offsets::SACRED_TREES_PERSISTENCE_FLAGS_TABLE;

    // Write sacred trees persistence flags table
    for(Entity* sacred_tree : sacred_trees_with_persistence)
    {
        uint8_t flag_byte = sacred_tree->persistence_flag().byte & 0xFF;
        uint8_t flag_bit = sacred_tree->persistence_flag().bit;

        rom.set_word(addr, sacred_tree->map()->id());
        addr += 0x2;
        rom.set_byte(addr, flag_byte);
        addr += 0x1;
        rom.set_byte(addr, flag_bit);
        addr += 0x1;
    }

    rom.set_word(addr, 0xFFFF);
    addr += 0x2;

    if(addr > offsets::SACRED_TREES_PERSISTENCE_FLAGS_TABLE_END)
        throw RandomizerException("Sacred trees persistence flags table must not be bigger than the one from base game");
    rom.mark_empty_chunk(addr, offsets::SACRED_TREES_PERSISTENCE_FLAGS_TABLE_END);
}
