#include "map.hpp"
#include "entity_on_map.hpp"

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
}

Map::~Map()
{
    for(EntityOnMap* entity_on_map : _entities)
        delete entity_on_map;
}

void Map::clear()
{
    _base_chest_id = 0x00;
    _fall_destination = 0xFFFF;
    _climb_destination = 0xFFFF;
    
    for(EntityOnMap* entity_on_map : _entities)
        delete entity_on_map;
    _entities.clear();
    _exits.clear();
    _variants.clear();
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
            _entities.push_back(EntityOnMap::from_rom(rom, addr, world));
    }
}

std::vector<uint8_t> Map::entities_as_bytes() const
{
    std::vector<uint8_t> bytes;

    for(EntityOnMap* entity : _entities)
    {
        std::vector<uint8_t> entity_bytes = entity->to_bytes();
        bytes.insert(bytes.end(), entity_bytes.begin(), entity_bytes.end());
    }

    bytes.push_back(0xFF);
    bytes.push_back(0xFF);
    return bytes;
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

Json Map::to_json() const
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
    json["unknownParam1"] = _unknown_param_1;
    json["unknownParam2"] = _unknown_param_2;

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

    if(!_variants.empty())
    {
        json["variants"] = Json::array();
        for(const MapVariant& variant : _variants)
            json["variants"].push_back(variant.to_json());
    }

    if(!_entities.empty())
    {
        json["entities"] = Json::array();
        uint8_t chest_id = _base_chest_id;
        for(EntityOnMap* entity : _entities)
        {
            Json entity_json = entity->to_json();
            if(entity_json.at("entityType") == "chest")
                entity_json["chestId"] = chest_id++;
            json["entities"].push_back(entity_json);
        }
    }


    return json;
}

Map* Map::from_json(const Json& json)
{
    // TODO
    return nullptr;
}