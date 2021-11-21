#include "map.hpp"
#include "entity.hpp"

#include "../exceptions.hpp"
#include "../world.hpp"
#include "../offsets.hpp"

Map::Map(uint16_t map_id, const md::ROM& rom, const World& world) :
    _id                 (map_id),
    _fall_destination   (0xFFFF),
    _climb_destination  (0xFFFF)
{
    this->read_map_data(rom);
    this->read_base_chest_id(rom);
    this->read_fall_destination(rom);
    this->read_climb_destination(rom);
    
    this->read_entities(rom, world);
    this->read_exits(rom);
    this->read_variants(rom);
    this->read_entity_masks(rom);
    this->read_clear_flags(rom);
    this->read_visited_flag(rom);
    this->read_dialogue_table(rom);
}

Map::Map(const Map& map) :
    _id                         (map._id),
    _address                    (map._address),
    _tileset_id                 (map._tileset_id),
    _primary_big_tileset_id     (map._primary_big_tileset_id),
    _secondary_big_tileset_id   (map._secondary_big_tileset_id),
    _big_tileset_id             (map._big_tileset_id),
    _palette_id                 (map._palette_id),
    _room_height                (map._room_height),
    _background_music           (map._background_music),
    _unknown_param_1            (map._unknown_param_1),
    _unknown_param_2            (map._unknown_param_2),
    _base_chest_id              (map._base_chest_id),
    _fall_destination           (map._fall_destination),
    _climb_destination          (map._climb_destination)
{
    for(Entity* entity : map._entities)
        this->add_entity(new Entity(*entity));

    for(const MapExit& exit : map._exits)
        _exits.push_back(exit);    

    for(const MapVariant& variant : map._variants)
        _variants.push_back(variant);

    for(const GlobalEntityMaskFlag& global_entity_mask_flag : map._global_entity_mask_flags)
        _global_entity_mask_flags.push_back(global_entity_mask_flag);
}

Map::~Map()
{
    for(Entity* entity : _entities)
        delete entity;
}

void Map::clear()
{
    _base_chest_id = 0x00;
    _fall_destination = 0xFFFF;
    _climb_destination = 0xFFFF;
    _entities.clear();
    _exits.clear();
    _variants.clear();
    _global_entity_mask_flags.clear();
}

void Map::write_to_rom(md::ROM& rom)
{
    this->write_map_data(rom);
    this->write_base_chest_id(rom);
    this->write_fall_destination(rom);
    this->write_climb_destination(rom);
}

////////////////////////////////////////////////////////////////

void Map::read_map_data(const md::ROM& rom)
{
    uint32_t addr = offsets::MAP_DATA_TABLE + (_id * 8);

    _address = rom.get_long(addr);

    _tileset_id = rom.get_byte(addr+4) & 0x1F;
    _primary_big_tileset_id = (rom.get_byte(addr+4) >> 5) & 0x01;
    _unknown_param_1 = (rom.get_byte(addr+4) >> 6);

    _palette_id = rom.get_byte(addr+5) & 0x3F;
    _unknown_param_2 = (rom.get_byte(addr+5) >> 6);

    _room_height = rom.get_byte(addr+6);

    _background_music = rom.get_byte(addr+7) & 0x1F;
    _secondary_big_tileset_id = (rom.get_byte(addr+7) >> 5) & 0x07;

    _big_tileset_id = _primary_big_tileset_id << 5 | _tileset_id;
}

void Map::write_map_data(md::ROM& rom)
{
    uint32_t addr = offsets::MAP_DATA_TABLE + (_id * 8);

    uint8_t byte4;
    byte4 = _tileset_id & 0x1F;
    byte4 |= (_primary_big_tileset_id & 0x01) << 5;
    byte4 |= (_unknown_param_1 & 0x03) << 6;
    
    uint8_t byte5;
    byte5 = _palette_id & 0x3F;
    byte5 |= (_unknown_param_2 & 0x03) << 6;
    
    uint8_t byte7;
    byte7 = _background_music & 0x1F;
    byte7 |= (_secondary_big_tileset_id & 0x07) << 5;
    
    rom.set_long(addr, _address);
    rom.set_byte(addr+4, byte4);
    rom.set_byte(addr+5, byte5);
    rom.set_byte(addr+6, _room_height);
    rom.set_byte(addr+7, byte7);
}

////////////////////////////////////////////////////////////////

void Map::read_base_chest_id(const md::ROM& rom)
{
    _base_chest_id = rom.get_byte(offsets::MAP_BASE_CHEST_ID_TABLE + _id);
}

void Map::write_base_chest_id(md::ROM& rom)
{
    rom.set_byte(offsets::MAP_BASE_CHEST_ID_TABLE + _id, _base_chest_id);
}

////////////////////////////////////////////////////////////////

void Map::read_fall_destination(const md::ROM& rom)
{
    for(uint32_t addr = offsets::MAP_FALL_DESTINATION_TABLE ; rom.get_word(addr) != 0xFFFF ; addr += 0x4)
    {
        if(rom.get_word(addr) == _id)
        {
            _fall_destination = rom.get_word(addr+2);
            return;
        }
    }
}

void Map::write_fall_destination(md::ROM& rom)
{
    if(_fall_destination == 0xFFFF)
        return;

    for(uint32_t addr = offsets::MAP_FALL_DESTINATION_TABLE ; rom.get_word(addr) != 0xFFFF ; addr += 0x4)
    {
        if(rom.get_word(addr) == _id)
        {
            rom.set_word(addr+2, _fall_destination);
            return;
        }
    }

    throw RandomizerException("Couldn't write fall destination for map #" + std::to_string(_id) + " because it had no fall destination in base game");
}

////////////////////////////////////////////////////////////////

void Map::read_climb_destination(const md::ROM& rom)
{
    for(uint32_t addr = offsets::MAP_CLIMB_DESTINATION_TABLE ; rom.get_word(addr) != 0xFFFF ; addr += 0x4)
    {
        if(rom.get_word(addr) == _id)
        {
            _climb_destination = rom.get_word(addr+2);
            return;
        }
    }
}

void Map::write_climb_destination(md::ROM& rom)
{
    if(_climb_destination == 0xFFFF)
        return;

    for(uint32_t addr = offsets::MAP_CLIMB_DESTINATION_TABLE ; rom.get_word(addr) != 0xFFFF ; addr += 0x4)
    {
        if(rom.get_word(addr) == _id)
        {
            rom.set_word(addr+2, _climb_destination);
            return;
        }
    }
}

////////////////////////////////////////////////////////////////

void Map::read_entities(const md::ROM& rom, const World& world)
{
    uint16_t offset = rom.get_word(offsets::MAP_ENTITIES_OFFSETS_TABLE + (_id*2));
    if(offset > 0)
    {
        // Maps with offset 0000 have no entities
        for(uint32_t addr = offsets::MAP_ENTITIES_TABLE + offset-1 ; rom.get_word(addr) != 0xFFFF ; addr += 0x8)
            this->add_entity(Entity::from_rom(rom, addr, this));
    }
}

std::vector<uint8_t> Map::entities_as_bytes() const
{
    std::vector<uint8_t> bytes;

    for(Entity* entity : _entities)
    {
        std::vector<uint8_t> entity_bytes = entity->to_bytes();
        bytes.insert(bytes.end(), entity_bytes.begin(), entity_bytes.end());
    }

    bytes.push_back(0xFF);
    bytes.push_back(0xFF);
    return bytes;
}

uint8_t Map::add_entity(Entity* entity) 
{
    entity->map(this);
    _entities.push_back(entity);
    return (uint8_t)_entities.size()-1;
}

void Map::insert_entity(uint8_t entity_id, Entity* entity) 
{
    entity->map(this);
    _entities.insert(_entities.begin() + entity_id, entity);

    // Shift entity indexes in clear flags
    for(uint8_t i=0 ; i<_global_entity_mask_flags.size() ; ++i)
    {
        GlobalEntityMaskFlag& global_mask_flag = _global_entity_mask_flags[i];
        if(entity_id <= global_mask_flag.first_entity_id)
            global_mask_flag.first_entity_id++;
    }
}

uint8_t Map::entity_id(const Entity* entity) const
{
    for(uint8_t id=0 ; id < _entities.size() ; ++id)
        if(_entities[id] == entity)
            return id;
    throw RandomizerException("Prompting entity ID of entity not inside map.");
}

void Map::remove_entity(uint8_t entity_id, bool delete_pointer) 
{ 
    Entity* erased_entity = _entities.at(entity_id);
    _entities.erase(_entities.begin() + entity_id);

    // If any other entity were using tiles from this entity, clear that
    for(Entity* entity : _entities)
    {
        if(entity->entity_to_use_tiles_from() == erased_entity)
            entity->entity_to_use_tiles_from(nullptr);
    }

    // Shift entity indexes in clear flags
    for(uint8_t i=0 ; i<_global_entity_mask_flags.size() ; ++i)
    {
        GlobalEntityMaskFlag& global_mask_flag = _global_entity_mask_flags[i];
        if(global_mask_flag.first_entity_id >= _entities.size())
        {
            _global_entity_mask_flags.erase(_global_entity_mask_flags.begin() + i);
            --i;
        }
        else if(global_mask_flag.first_entity_id > entity_id)
            global_mask_flag.first_entity_id--;
    }

    if(delete_pointer)
        delete erased_entity;
}

void Map::move_entity(uint8_t entity_id, uint8_t entity_new_id)
{
    Entity* entity_to_move = _entities.at(entity_id); 
    this->remove_entity(entity_id, false);
    this->insert_entity(entity_new_id, entity_to_move);
} 

void Map::clear_entities()
{
    _entities.clear();
}

////////////////////////////////////////////////////////////////

void Map::read_exits(const md::ROM& rom)
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

        if(_id == map_id_1)
        {
            MapExit exit;
            exit.pos_x = pos_x_1;
            exit.pos_y = pos_y_1;
            exit.extra_byte = extra_1;
            exit.destination_map_id = map_id_2;
            exit.destination_x = pos_x_2;
            exit.destination_y = pos_y_2;
            _exits.push_back(exit);
        }
        else if(_id == map_id_2)
        {
            MapExit exit;
            exit.pos_x = pos_x_2;
            exit.pos_y = pos_y_2;
            exit.extra_byte = extra_2;
            exit.destination_map_id = map_id_1;
            exit.destination_x = pos_x_1;
            exit.destination_y = pos_y_1;
            _exits.push_back(exit);
        }
    }
}

////////////////////////////////////////////////////////////////

void Map::read_variants(const md::ROM& rom)
{
    for(uint32_t addr = offsets::MAP_VARIANTS_TABLE ; rom.get_word(addr) != 0xFFFF ; addr += 0x6)
    {
        if(rom.get_word(addr) == _id)
        {
            MapVariant variant;
            variant.map_variant_id = rom.get_word(addr+2);
            variant.flag_byte = rom.get_byte(addr+4);
            variant.flag_bit = rom.get_byte(addr+5);
            _variants.push_back(variant);
        }
    } 

}

////////////////////////////////////////////////////////////////

void Map::read_entity_masks(const md::ROM& rom)
{
    for(uint32_t addr = offsets::MAP_ENTITY_MASKS_TABLE ; rom.get_word(addr) != 0xFFFF ; addr += 0x4)
    {
        if(rom.get_word(addr) != _id)
            continue;

        uint16_t word = rom.get_word(addr+2);

        uint8_t msb = word >> 8;
        uint8_t lsb = word & 0x00FF;

        bool visibility_if_flag_set = msb >> 7;
        uint8_t flag_byte = msb & 0x7F;
        uint8_t flag_bit = lsb >> 5;
        uint8_t entity_id = lsb & 0x0F;

        Entity* entity = _entities.at(entity_id);
        entity->mask_flags().push_back(EntityMaskFlag(visibility_if_flag_set, flag_byte, flag_bit));
    }
}

std::vector<uint8_t> Map::entity_masks_as_bytes() const
{
    std::vector<uint8_t> bytes; 

    uint8_t map_id_msb = (_id >> 8) & 0x00FF;
    uint8_t map_id_lsb = _id & 0x00FF;

    for(uint8_t entity_id=0 ; entity_id<_entities.size() ; ++entity_id)
    {
        Entity* entity = _entities.at(entity_id);
        for(EntityMaskFlag& mask_flag : entity->mask_flags())
        {
            uint8_t flag_msb = mask_flag.byte & 0x7F;
            if(mask_flag.visibility_if_flag_set)
                flag_msb |= 0x80;

            uint8_t flag_lsb = entity_id & 0x0F;
            flag_lsb |= (mask_flag.bit & 0x7) << 5;

            bytes.push_back(map_id_msb);
            bytes.push_back(map_id_lsb);
            bytes.push_back(flag_msb);
            bytes.push_back(flag_lsb);
        }
    }

    return bytes;
}

////////////////////////////////////////////////////////////////

void Map::read_clear_flags(const md::ROM& rom)
{
    for(uint32_t addr = offsets::MAP_CLEAR_FLAGS_TABLE ; rom.get_word(addr) != 0xFFFF ; addr += 0x4)
    {
        if(rom.get_word(addr) == _id)
        {
            uint16_t word = rom.get_word(addr+2);
            uint8_t msb = word >> 8;
            uint8_t lsb = word & 0x00FF;

            uint8_t flag_byte = msb;
            uint8_t flag_bit = lsb >> 5;
            uint8_t first_entity_id = lsb & 0x1F;
            
            _global_entity_mask_flags.push_back(GlobalEntityMaskFlag(flag_byte, flag_bit, first_entity_id));
        }
    }
}

////////////////////////////////////////////////////////////////

void Map::read_visited_flag(const md::ROM& rom)
{
    uint16_t flag_description = rom.get_word(offsets::MAP_VISITED_FLAG_TABLE + (_id * 2));
    uint16_t byte = (flag_description >> 3) + 0xC0;
    uint8_t bit = flag_description & 0x7;
    _visited_flag = Flag(byte, bit);
}

////////////////////////////////////////////////////////////////

void Map::read_dialogue_table(const md::ROM& rom)
{
    uint32_t addr = offsets::DIALOGUE_TABLE;

    // Build a table to know which entity uses which dialogue
    std::map<uint8_t, std::vector<Entity*>> talkable_entities;
    for(Entity* entity : _entities)
    {
        if(entity->talkable())
            talkable_entities[entity->dialogue()].push_back(entity);
    }

    // Read actual dialogue info to match entities with speaker ID
    uint16_t header_word = rom.get_word(addr);
    while(header_word != 0xFFFF)
    {
        uint16_t map_id = header_word & 0x7FF;
        uint8_t length = header_word >> 11;

        if(map_id == _id)
        {
            uint8_t current_npc_dialogue = 0;

            for(uint8_t i=0 ; i<length ; ++i)
            {
                uint32_t offset = (i+1)*2;
                uint16_t word = rom.get_word(addr + offset);
                
                uint16_t speaker_id = word & 0x7FF;
                uint8_t consecutive_speakers = word >> 11;
                for(uint8_t j=0 ; j<consecutive_speakers ; ++j)
                {
                    for(Entity* entity : talkable_entities[current_npc_dialogue++])
                        entity->speaker_id(speaker_id);
                    speaker_id++;
                }
            }

            return;
        }

        addr += (length + 1) * 2;
        header_word = rom.get_word(addr);
    }
}

////////////////////////////////////////////////////////////////

Json Map::to_json(const World& world) const
{
    Json json;

    json["address"] = _address;
    json["tilesetId"] = _tileset_id;
    json["primaryBigTilesetId"] = _primary_big_tileset_id;
    json["secondaryBigTilesetId"] = _secondary_big_tileset_id;
    json["bigTilesetId"] = _big_tileset_id;
    json["paletteId"] = _palette_id;
    json["roomHeight"] = _room_height;
    json["backgroundMusic"] = _background_music;
    json["baseChestId"] = _base_chest_id;

    if(_fall_destination != 0xFFFF)
        json["fallDestination"] = _fall_destination;
    if(_climb_destination != 0xFFFF)
        json["climbDestination"] = _climb_destination;

    if(!_exits.empty())
    {
        json["exits"] = Json::array();
        for(const MapExit& exit : _exits)
            json["exits"].push_back(exit.to_json());
    }

    json["visitedFlag"] = _visited_flag.to_json();

    if(!_variants.empty())
    {
        json["variants"] = Json::array();
        for(const MapVariant& variant : _variants)
            json["variants"].push_back(variant.to_json());
    }

    if(!_global_entity_mask_flags.empty())
    {
        json["globalEntityMaskFlags"] = Json::array();
        for(const GlobalEntityMaskFlag& global_mask_flag : _global_entity_mask_flags)
            json["globalEntityMaskFlags"].push_back(global_mask_flag.to_json());
    }

    if(!_entities.empty())
    {
        json["entities"] = Json::array();
        uint8_t chest_id = _base_chest_id;
        for(Entity* entity : _entities)
        {
            Json entity_json = entity->to_json(world);
            if(entity_json.at("entityType") == "chest")
                entity_json["chestId"] = chest_id++;
            json["entities"].push_back(entity_json);
        }
    }

    json["unknownParam1"] = _unknown_param_1;
    json["unknownParam2"] = _unknown_param_2;

    return json;
}
