#include "entity.hpp"

#include "../world.hpp"
#include "map.hpp"
#include "entity_type.hpp"

Entity::Entity() :
    _map                            (nullptr),
    _entity_type_id                 (0),
    _pos_x                          (0),
    _pos_y                          (0),
    _pos_z                          (0),
    _half_tile_x                    (false),
    _half_tile_y                    (false),
    _half_tile_z                    (false),
    _orientation                    (0),
    _palette                        (0),
    _speed                          (0),
    _fightable                      (false),
    _liftable                       (false),
    _talkable                       (false),
    _can_pass_through               (false),
    _appear_after_player_moved_away (false),
    _dialogue                       (0),
    _behavior_id                    (0),
    _entity_to_use_tiles_from       (nullptr),
    _flag_unknown_2_3               (false),
    _flag_unknown_2_4               (false),
    _flag_unknown_3_5               (false),
    _flag_unknown_6_7               (false)
{}

Entity::Entity(const Entity& entity) :
    _map                                (entity._map),
    _entity_type_id                     (entity._entity_type_id),
    _pos_x                              (entity._pos_x),
    _pos_y                              (entity._pos_y),
    _pos_z                              (entity._pos_z),
    _half_tile_x                        (entity._half_tile_x),
    _half_tile_y                        (entity._half_tile_y),
    _half_tile_z                        (entity._half_tile_z),
    _orientation                        (entity._orientation),
    _palette                            (entity._palette),
    _speed                              (entity._speed),
    _entity_to_use_tiles_from           (entity._entity_to_use_tiles_from),
    _dialogue                           (entity._dialogue),
    _fightable                          (entity._fightable),
    _liftable                           (entity._liftable),
    _talkable                           (entity._talkable),
    _can_pass_through                   (entity._can_pass_through),
    _appear_after_player_moved_away     (entity._appear_after_player_moved_away),
    _behavior_id                        (entity._behavior_id),
    _flag_unknown_2_3                   (entity._flag_unknown_2_3),
    _flag_unknown_2_4                   (entity._flag_unknown_2_4),
    _flag_unknown_3_5                   (entity._flag_unknown_3_5),
    _flag_unknown_6_7                   (entity._flag_unknown_6_7)
{}

uint8_t Entity::entity_id() const 
{ 
    return _map->entity_id(this);
}

Json Entity::to_json(const World& world) const
{
    Json json;

    json["entityType"] = world.entity_type(_entity_type_id)->name();

    json["posX"] = _pos_x;
    json["posY"] = _pos_y;
    json["posZ"] = _pos_z;

    json["halfTileX"] = _half_tile_x;
    json["halfTileY"] = _half_tile_y;
    json["halfTileZ"] = _half_tile_z;

    json["orientation"] = _orientation;
    json["palette"] = _palette;
    json["speed"] = _speed;

    json["fightable"] = _fightable;
    json["liftable"] = _liftable;
    json["talkable"] = _talkable;
    json["appearAfterPlayerMovedAway"] = _appear_after_player_moved_away;
    json["canPassThrough"] = _can_pass_through;

    json["dialogue"] = _dialogue;
    json["behaviorId"] = _behavior_id;

    bool use_tiles_from_other_entity = (_entity_to_use_tiles_from != nullptr);
    json["useTilesFromOtherEntity"] = use_tiles_from_other_entity;
    if(use_tiles_from_other_entity)
        json["entityIdToUseTilesFrom"] = _entity_to_use_tiles_from->entity_id();

    json["flagUnknown_2_3"] = _flag_unknown_2_3;
    json["flagUnknown_2_4"] = _flag_unknown_2_4;
//    json["flagUnknown_3_5"] = _flag_unknown_3_5;
    json["flagUnknown_6_7"] = _flag_unknown_6_7;

    if(!_mask_flags.empty())
    {
        json["maskFlags"] = Json::array();
        for(const EntityMaskFlag& mask : _mask_flags)
            json["maskFlags"].push_back(mask.to_json());
    }

    return json;
}

Entity* Entity::from_json(const Json& json, Map* map, const World& world)
{
    Entity* entity = new Entity();

    entity->_map = map;

    entity->_entity_type_id = world.entity_type((std::string)json.at("entityType"))->id();

    entity->_pos_x = json.at("posX");
    entity->_pos_y = json.at("posY");
    entity->_pos_z = json.at("posZ");

    entity->_half_tile_x = json.at("halfTileX");
    entity->_half_tile_y = json.at("halfTileY");
    entity->_half_tile_z = json.at("halfTileZ");

    entity->_orientation = json.at("orientation");
    entity->_palette = json.at("palette");
    entity->_speed = json.at("speed");

    entity->_fightable = json.at("fightable");
    entity->_liftable = json.at("liftable");
    entity->_talkable = json.at("talkable");
    entity->_appear_after_player_moved_away = json.at("appearAfterPlayerMovedAway");
    entity->_can_pass_through = json.at("canPassThrough");

    entity->_dialogue = json.at("dialogue");
    entity->_behavior_id = json.at("behaviorId");

    bool use_tiles_from_other_entity = json.at("useTilesFromOtherEntity");
    if(use_tiles_from_other_entity)
    {
        uint8_t entity_id_to_use_tiles_from = json.at("entityIdToUseTilesFrom");
        entity->_entity_to_use_tiles_from = map->entity(entity_id_to_use_tiles_from);
    }

    entity->_flag_unknown_2_3 = json.at("flagUnknown_2_3");
    entity->_flag_unknown_2_4 = json.at("flagUnknown_2_4");
//    entity->_flag_unknown_3_5 = json.at("flagUnknown_3_5");
    entity->_flag_unknown_6_7 = json.at("flagUnknown_6_7");

    if(json.contains("maskFlags"))
    {
        for(const Json& j : json.at("maskFlags"))
            entity->_mask_flags.push_back(EntityMaskFlag::from_json(j));
    }

    return entity;
}

Entity* Entity::from_rom(const md::ROM& rom, uint32_t addr, Map* map)
{
    Entity* entity = new Entity();

    entity->_map = map;

    // Byte 0
    uint8_t byte0 = rom.get_byte(addr);
    entity->_orientation = (byte0 & 0xC0) >> 6;
    entity->_pos_x = byte0 & 0x3F;

    // Byte 1
    uint8_t byte1 = rom.get_byte(addr+1);
    entity->_palette = (byte1 & 0xC0) >> 6;
    entity->_pos_y = byte1 & 0x3F;

    // Byte 2
    uint8_t byte2 = rom.get_byte(addr+2);
    entity->_fightable = byte2 & 0x80;
    entity->_liftable = byte2 & 0x40;
    entity->_talkable = byte2 & 0x20;
    entity->_flag_unknown_2_4 = byte2 & 0x10;
    entity->_flag_unknown_2_3 = byte2 & 0x08;
    entity->_speed = byte2 & 0x07;

    // Byte 3
    uint8_t byte3 = rom.get_byte(addr+3);
    entity->_half_tile_x = byte3 & 0x80;
    entity->_half_tile_y = byte3 & 0x40;
    entity->_flag_unknown_3_5 = byte3 & 0x20;
    
    bool use_tiles_from_other_entity = byte3 & 0x10;
    if(use_tiles_from_other_entity)
    {
        uint8_t entity_id_to_use_tiles_from = byte3 & 0x0F;
        // There are a few occurences in the game where an entity points at itself on this property...
        if(entity_id_to_use_tiles_from == map->entities().size())
            entity->_entity_to_use_tiles_from = entity;
        else
            entity->_entity_to_use_tiles_from = map->entity(entity_id_to_use_tiles_from);
    }

    // Byte 4
    uint8_t byte4 = rom.get_byte(addr+4);
    entity->_dialogue = (byte4 & 0xFC) >> 2;

    // Byte 5
    entity->_entity_type_id = rom.get_byte(addr+5);

    // Byte 6
    uint8_t byte6 = rom.get_byte(addr+6);
    entity->_pos_z = byte6 & 0x0F;
    entity->_can_pass_through = byte6 & 0x10;
    entity->_appear_after_player_moved_away = byte6 & 0x20;
    entity->_half_tile_z = byte6 & 0x40;
    entity->_flag_unknown_6_7 = byte6 & 0x80;

    // Byte 7
    uint8_t byte7 = rom.get_byte(addr+7);
    entity->_behavior_id = byte7;
    entity->_behavior_id |= (byte4 & 0x03) << 8;

    return entity;
}

std::vector<uint8_t> Entity::to_bytes() const
{
    // Byte 0
    uint8_t byte0 = _pos_x & 0x3F;
    byte0 |= (_orientation & 0x3) << 6;

    // Byte 1
    uint8_t byte1 = _pos_y & 0x3F;
    byte1 |= ((_palette & 0x3) << 6);

    // Byte 2
    uint8_t byte2 = _speed & 0x07;
    if(_fightable)          byte2 |= 0x80;
    if(_liftable)           byte2 |= 0x40;
    if(_talkable)           byte2 |= 0x20;
    if(_flag_unknown_2_4)   byte2 |= 0x10;
    if(_flag_unknown_2_3)   byte2 |= 0x08;

    // Byte 3
    bool use_tiles_from_other_entity = (_entity_to_use_tiles_from != nullptr);
    uint8_t entity_id_to_use_tiles_from = 0;
    if(_entity_to_use_tiles_from)
        entity_id_to_use_tiles_from = _entity_to_use_tiles_from->entity_id();

    uint8_t byte3 = entity_id_to_use_tiles_from & 0x0F;
    if(_half_tile_x)                 byte3 |= 0x80;
    if(_half_tile_y)                 byte3 |= 0x40;
    if(_flag_unknown_3_5)            byte3 |= 0x20;
    if(use_tiles_from_other_entity)  byte3 |= 0x10;

    // Byte 4
    uint8_t byte4 = (_dialogue & 0x3F) << 2;
    byte4 |= (_behavior_id >> 8) & 0x3;

    // Byte 5
    uint8_t byte5 = _entity_type_id;

    // Byte 6
    uint8_t byte6 = _pos_z & 0x0F;
    if(_can_pass_through)   byte6 |= 0x10;
    if(_appear_after_player_moved_away)   byte6 |= 0x20;
    if(_half_tile_z)        byte6 |= 0x40;
    if(_flag_unknown_6_7)   byte6 |= 0x80;

    // Byte 7
    uint8_t byte7 = _behavior_id & 0x00FF;

    return { byte0, byte1, byte2, byte3, byte4, byte5, byte6, byte7 };
}
