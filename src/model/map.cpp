#include "map.hpp"
#include "entity_on_map.hpp"

#include "../exceptions.hpp"
#include "../world.hpp"

Map::Map(uint16_t map_id, const md::ROM& rom, const World& world)
{
    _id = map_id;

    // Read map data from the global table
    constexpr uint32_t MAP_DATA_TABLE_ADDR = 0xA0A12;
    uint32_t addr = MAP_DATA_TABLE_ADDR + (map_id * 8);

    _address = rom.get_long(addr);

    _tileset_id = rom.get_byte(addr+4) & 0x1F;
    _primary_big_tileset_id = (rom.get_byte(addr+4) >> 5) & 0x01;
    _secondary_big_tileset_id = (rom.get_byte(addr+7) >> 5) & 0x07;
    _big_tileset_id = _primary_big_tileset_id << 5 | _tileset_id;

    _palette_id = rom.get_byte(addr+5) & 0x3F;
    _room_height = rom.get_byte(addr+6);
    _background_music = rom.get_byte(addr+7) & 0x1F;

    _unknown_param_1 = (rom.get_byte(addr+4) >> 6);
    _unknown_param_2 = (rom.get_byte(addr+5) >> 6);

    // Read base chest ID for this map from the dedicated table
    constexpr uint32_t MAP_BASE_CHEST_ID_TABLE_ADDR = 0x9E78E;
    _base_chest_id = rom.get_byte(MAP_BASE_CHEST_ID_TABLE_ADDR + map_id);

    // Read fall destination for this map from the dedicated table
    constexpr uint32_t MAP_FALL_DESTINATION_TABLE_ADDR = 0xA1A8;
    _fall_destination = 0xFFFF;
    for(addr = MAP_FALL_DESTINATION_TABLE_ADDR ; rom.get_word(addr) != 0xFFFF ; addr += 0x4)
    {
        if(rom.get_word(addr) == _id)
        {
            _fall_destination = rom.get_word(addr+2);
            break;
        }
    }

    // Read climb destination for this map from the dedicated table
    constexpr uint32_t MAP_CLIMB_DESTINATION_TABLE_ADDR = 0xA35A;
    _climb_destination = 0xFFFF;
    for(addr = MAP_CLIMB_DESTINATION_TABLE_ADDR ; rom.get_word(addr) != 0xFFFF ; addr += 0x4)
    {
        if(rom.get_word(addr) == _id)
        {
            _climb_destination = rom.get_word(addr+2);
            break;
        }
    }

    // Read entities in this map from the dedicated table
    constexpr uint32_t MAP_ENTITIES_OFFSETS_TABLE_ADDR = 0x1B090;
    constexpr uint32_t MAP_ENTITIES_TABLE_ADDR = 0x1B932;
    uint16_t offset = rom.get_word(MAP_ENTITIES_OFFSETS_TABLE_ADDR + (map_id*2));
    if(offset > 0)
    {
        // Maps with offset 0000 have no entities
        for(addr = MAP_ENTITIES_TABLE_ADDR + offset-1 ; rom.get_word(addr) != 0xFFFF ; addr += 0x8)
            _entities.push_back(EntityOnMap::from_rom(rom, addr, world));
    }

    // Read map exits from the dedicated table
    constexpr uint32_t MAP_EXITS_TABLE_ADDR = 0x11CEA2;
    for(addr = MAP_EXITS_TABLE_ADDR ; rom.get_word(addr) != 0xFFFF ; addr += 0x8)
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

    // Read map variants for this map from the dedicated table
    constexpr uint32_t MAP_VARIANTS_TABLE_ADDR = 0xA3D8;
    for(addr = MAP_VARIANTS_TABLE_ADDR ; rom.get_word(addr) != 0xFFFF ; addr += 0x6)
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

Map::~Map()
{
    for(EntityOnMap* entity_on_map : _entities)
        delete entity_on_map;
}

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