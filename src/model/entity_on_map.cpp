#include "entity_on_map.hpp"

#include "../world.hpp"
#include "entity_type.hpp"

Json EntityOnMap::to_json(const World& world) const
{
    Json json;

    json["entityType"] = world.entity_type(_entity_type_id)->name();

    json["posX"] = _pos_x;
    json["posY"] = _pos_y;
    json["posZ"] = _pos_z;

    json["halfTileX"] = _half_tile_x;
    json["halfTileY"] = _half_tile_y;
    json["halfTileZ"] = _half_tile_z;

    json["palette"] = _palette;

    json["fightable"] = _fightable;
    json["liftable"] = _liftable;
    json["talkable"] = _talkable;
    json["horizontalMirror"] = _horizontal_mirror;
    json["appearAfterPlayerMovedAway"] = _appear_after_player_moved_away;
    json["canPassThrough"] = _can_pass_through;

    json["flagUnknown_0_7"] = _flag_unknown_0_7;
    json["flagUnknown_6_7"] = _flag_unknown_6_7;

    json["byte2Remainder"] = _byte2_remainder;
    json["byte3Remainder"] = _byte3_remainder;
    json["byte4"] = _byte4;
    json["byte7"] = _byte7; 

    return json;
}

EntityOnMap* EntityOnMap::from_json(const Json& json, Map* map, const World& world)
{
    EntityOnMap* entity = new EntityOnMap(map);

    entity->_pos_x = json.at("posX");
    entity->_pos_y = json.at("posY");
    entity->_pos_z = json.at("posZ");

    entity->_half_tile_x = json.at("halfTileX");
    entity->_half_tile_y = json.at("halfTileY");
    entity->_half_tile_z = json.at("halfTileZ");

    entity->_entity_type_id = world.entity_type((std::string)json.at("entityType"))->id();
    entity->_palette = json.at("palette");

    entity->_horizontal_mirror = json.at("horizontalMirror");
    entity->_flag_unknown_0_7 = json.at("flagUnknown_0_7");

    entity->_fightable = json.at("fightable");
    entity->_liftable = json.at("liftable");
    entity->_talkable = json.at("talkable");
    entity->_byte2_remainder = json.at("byte2Remainder");

    entity->_can_pass_through = json.at("canPassThrough");
    entity->_appear_after_player_moved_away = json.at("appearAfterPlayerMovedAway");
    entity->_flag_unknown_6_7 = json.at("flagUnknown_6_7");

    entity->_byte3_remainder = json.at("byte3Remainder");
    entity->_byte4 = json.at("byte4");
    entity->_byte7 = json.at("byte7");

    return entity;
}

EntityOnMap* EntityOnMap::from_rom(const md::ROM& rom, uint32_t addr, Map* map, const World& world)
{
    EntityOnMap* entity = new EntityOnMap(map);
    
    // Byte 0
    uint8_t byte0 = rom.get_byte(addr);
    entity->_flag_unknown_0_7 = byte0 & 0x80;
    entity->_horizontal_mirror = byte0 & 0x40;
    entity->_pos_x = byte0 & 0x3F;

    // Byte 1
    uint8_t byte1 = rom.get_byte(addr + 0x1);
    entity->_palette = (byte1 & 0xC0) >> 6;
    entity->_pos_y = byte1 & 0x3F;

    // Byte 2
    uint8_t byte2 = rom.get_byte(addr + 0x2);
    entity->_fightable = byte2 & 0x80;
    entity->_liftable = byte2 & 0x40;
    entity->_talkable = byte2 & 0x20;
    entity->_byte2_remainder = byte2 & 0x1F;

    // Byte 3
    uint8_t byte3 = rom.get_byte(addr + 0x3);
    entity->_half_tile_x = byte3 & 0x80;
    entity->_half_tile_y = byte3 & 0x40;
    entity->_byte3_remainder = byte3 & 0x3F;

    // Byte 4
    entity->_byte4 = rom.get_byte(addr + 0x4);

    // Byte 5
    entity->_entity_type_id = rom.get_byte(addr + 0x5);

    // Byte 6
    uint8_t byte6 = rom.get_byte(addr + 0x6);
    entity->_pos_z = byte6 & 0x0F;
    entity->_can_pass_through = byte6 & 0x10;
    entity->_appear_after_player_moved_away = byte6 & 0x20;
    entity->_half_tile_z = byte6 & 0x40;
    entity->_flag_unknown_6_7 = byte6 & 0x80;

    // Byte 7
    uint8_t byte7 = rom.get_byte(addr + 0x7);
    entity->_byte7 = byte7;

    return entity;
}

std::vector<uint8_t> EntityOnMap::to_bytes() const
{
    // Byte 0
    uint8_t byte0 = _pos_x & 0x3F;
    if(_flag_unknown_0_7)
        byte0 |= 0x80;
    if(_horizontal_mirror)
        byte0 |= 0x40;

    // Byte 1
    uint8_t byte1 = _pos_y & 0x3F;
    byte1 |= ((_palette & 0x3) << 6);

    // Byte 2
    uint8_t byte2 = _byte2_remainder & 0x1F;
    if(_fightable)  byte2 |= 0x80;
    if(_liftable)   byte2 |= 0x40;
    if(_talkable)   byte2 |= 0x20;

    // Byte 3
    uint8_t byte3 = _byte3_remainder & 0x3F;
    if(_half_tile_x)   byte3 |= 0x80;
    if(_half_tile_y)   byte3 |= 0x40;

    // Byte 4
    uint8_t byte4 = _byte4;

    // Byte 5
    uint8_t byte5 = _entity_type_id;

    // Byte 6
    uint8_t byte6 = _pos_z & 0x0F;
    if(_can_pass_through)   byte6 |= 0x10;
    if(_appear_after_player_moved_away)   byte6 |= 0x20;
    if(_half_tile_z)        byte6 |= 0x40;
    if(_flag_unknown_6_7)   byte6 |= 0x80;

    // Byte 7
    uint8_t byte7 = _byte7;

    return { byte0, byte1, byte2, byte3, byte4, byte5, byte6, byte7 };
}