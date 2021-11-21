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
    write_entity_types(rom, world);
    write_dialogue_table(rom, world);
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

void WorldWriter::write_dialogue_table(md::ROM& rom, const World& world)
{
    uint32_t addr = offsets::DIALOGUE_TABLE;

    for(auto& [map_id, map] : world.maps())
    {
        std::vector<Entity*> sorted_entities = map->entities();
        std::sort(sorted_entities.begin(), sorted_entities.end(), [](Entity* e1, Entity* e2) { return e1->speaker_id() < e2->speaker_id(); });
       
        std::vector<std::pair<uint16_t, uint8_t>> consecutive_packs;

        uint16_t previous_speaker_id = 0xFFFE;
        uint8_t current_dialogue_id = -1;
        // Assign "dialogue" sequentially to entities
        for(Entity* entity : sorted_entities)
        {
            if(!entity->talkable())
                continue;

            if(entity->speaker_id() != previous_speaker_id)
            {
                if(entity->speaker_id() == previous_speaker_id + 1)
                    consecutive_packs[consecutive_packs.size()-1].second++;
                else
                    consecutive_packs.push_back(std::make_pair(entity->speaker_id(), 1));

                previous_speaker_id = entity->speaker_id();
                current_dialogue_id++;
            }
            
            entity->dialogue(current_dialogue_id);
        }

        if(consecutive_packs.empty())
            continue;

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

    rom.set_word(addr, 0xFFFF);
    addr += 0x2;
    if(addr > offsets::DIALOGUE_TABLE_END)
        throw RandomizerException("Dialogue table is bigger than in original game");
    rom.mark_empty_chunk(addr, offsets::DIALOGUE_TABLE_END);
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
        std::vector<uint8_t> entity_masks_bytes = map->entity_masks_as_bytes();
        rom.set_bytes(entity_masks_table_current_addr, entity_masks_bytes);
        entity_masks_table_current_addr += (uint32_t)entity_masks_bytes.size();

        // Write global entity mask flags
        for(const GlobalEntityMaskFlag& global_mask_flags : map->global_entity_mask_flags())
        {
            uint16_t flag_bytes = global_mask_flags.to_bytes();
            rom.set_word(clear_flags_table_current_addr, map->id());
            rom.set_word(clear_flags_table_current_addr+2, flag_bytes);
            clear_flags_table_current_addr += 0x4;
        }

        // Write map visited flag
        const Flag& visited_flag = map->visited_flag();
        uint16_t flag_word = (visited_flag.byte << 3) + visited_flag.bit;
        rom.set_word(offsets::MAP_VISITED_FLAG_TABLE + (map->id()*2), flag_word);
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
