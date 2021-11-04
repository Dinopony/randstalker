#include "entity_on_map.hpp"

#include "../world.hpp"
#include "entity.hpp"

Json EntityOnMap::to_json() const
{
    Json json;

    json["posX"] = _pos_x;
    json["posY"] = _pos_y;
    json["posZ"] = _pos_z;

    json["entityType"] = _entity_type->name();
    json["palette"] = _palette;

    json["horizontal_mirror"] = _horizontal_mirror;
    json["flagUnknown_0_7"] = _flag_unknown_0_7;
    json["fightable"] = _fightable;
    json["liftable"] = _liftable;

    json["byte2Remainder"] = _byte2_remainder;
    json["byte3"] = _byte3;
    json["byte4"] = _byte4;
    json["byte6MSH"] = _byte6_msh;
    json["byte7"] = _byte7;

    return json;
}

EntityOnMap* EntityOnMap::from_json(const Json& json, const World& world)
{
    EntityOnMap* entity = new EntityOnMap();

    entity->_pos_x = json.at("posX");
    entity->_pos_y = json.at("posY");
    entity->_pos_z = json.at("posZ");

    entity->_entity_type = world.entity((std::string)json.at("entityType"));
    entity->_palette = json.at("palette");

    entity->_horizontal_mirror = json.at("horizontal_mirror");
    entity->_flag_unknown_0_7 = json.at("flagUnknown_0_7");
    entity->_fightable = json.at("fightable");
    entity->_liftable = json.at("liftable");

    entity->_byte2_remainder = json.at("byte2Remainder");
    entity->_byte3 = json.at("byte3");
    entity->_byte4 = json.at("byte4");
    entity->_byte6_msh = json.at("byte6MSH");
    entity->_byte7 = json.at("byte7");

    return entity;
}

EntityOnMap* EntityOnMap::from_rom(const md::ROM& rom, uint32_t addr, const World& world)
{
    EntityOnMap* entity = new EntityOnMap();
    
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
    entity->_byte2_remainder = byte2 & 0x3F;

    // Byte 3
    uint8_t byte3 = rom.get_byte(addr + 0x3);
    entity->_byte3 = byte3;

    // Byte 4
    entity->_byte4 = rom.get_byte(addr + 0x4);

    // Byte 5
    entity->_entity_type = world.entity(rom.get_byte(addr + 0x5));

    // Byte 6
    uint8_t byte6 = rom.get_byte(addr + 0x6);
    entity->_pos_z = byte6 & 0x0F;
    entity->_byte6_msh = (byte6 & 0xF0) >> 4;

    // Byte 7
    uint8_t byte7 = rom.get_byte(addr + 0x7);
    entity->_byte7 = byte7;

    return entity;
}

std::vector<uint8_t> EntityOnMap::to_bytes() const
{
    // Byte 0
    uint8_t byte0 = _pos_x;
    if(_flag_unknown_0_7)
        byte0 |= 0x80;
    if(_horizontal_mirror)
        byte0 |= 0x40;

    // Byte 1
    uint8_t byte1 = _pos_y;
    byte1 |= ((_palette & 0x3) << 6);

    // Byte 2
    uint8_t byte2 = _byte2_remainder;
    if(_fightable)
        byte2 |= 0x80;
    if(_liftable)
        byte2 |= 0x40;

    // Byte 3
    uint8_t byte3 = _byte3;

    // Byte 4
    uint8_t byte4 = _byte4;

    // Byte 5
    uint8_t byte5 = _entity_type->id();

    // Byte 6
    uint8_t byte6 = _pos_z;
    byte6 |= (_byte6_msh << 4);

    // Byte 7
    uint8_t byte7 = _byte7;

    return { byte0, byte1, byte2, byte3, byte4, byte5, byte6, byte7 };
}