#include "entity.hpp"

#include "world.hpp"
#include "map.hpp"
#include "entity_type.hpp"
#include <set>

#include "../constants/entity_type_codes.hpp"

static std::map<uint8_t, uint8_t> DEFAULT_PALETTES = {
    { ENTITY_SMALL_YELLOW_PLATFORM, 2 },
    { ENTITY_SMALL_YELLOW_PLATFORM_ALTERNATE, 2 },
    { ENTITY_SMALL_THIN_YELLOW_PLATFORM, 2 },
    { ENTITY_LARGE_THIN_YELLOW_PLATFORM, 2 },
    { ENTITY_SMALL_GREEN_PLATFORM, 2 },
    { ENTITY_LARGE_THIN_GREEN_PLATFORM, 2 },
    { ENTITY_DEAD_BODY, 2 },
    { ENTITY_INJURED_DOG, 2 },
    { ENEMY_SKELETON_1, 2 },
    { ENTITY_LARGE_BLUE_BOULDER, 2 },
    { ENTITY_CHEST, 2 },
    { ENTITY_VASE, 2 },
    { ENTITY_CRATE, 2 }
};

static std::set<uint8_t> LIFTABLE_TYPES = {
    ENTITY_CRATE, ENTITY_VASE, ENTITY_BUTTON_CUBE
};

static std::set<uint8_t> FIGHTABLE_TYPES = {
    ENTITY_SMALL_GRAY_SPIKEBALL,
    ENTITY_LARGE_GRAY_SPIKEBALL,
    ENTITY_SACRED_TREE
};

static std::set<uint8_t> ENEMY_TYPES = {
    ENEMY_SLIME_1, ENEMY_SLIME_2, ENEMY_SLIME_3, ENEMY_SLIME_4, ENEMY_SLIME_5, ENEMY_SLIME_6,
    ENEMY_ORC_1, ENEMY_ORC_2, ENEMY_ORC_3,
    ENEMY_WORM_1, ENEMY_WORM_2, ENEMY_WORM_3,
    ENEMY_MUSHROOM_1, ENEMY_MUSHROOM_2, ENEMY_MUSHROOM_3,
    ENEMY_GIANT_1, ENEMY_GIANT_2, ENEMY_GIANT_3,
    ENEMY_REAPER_1, ENEMY_REAPER_2, ENEMY_REAPER_3,
    ENEMY_GOLEM_1, ENEMY_GOLEM_2, ENEMY_GOLEM_3,
    ENEMY_SPECTRE_1, ENEMY_SPECTRE_2, ENEMY_SPECTRE_3,
    ENEMY_NINJA_1, ENEMY_NINJA_2, ENEMY_NINJA_3,
    ENEMY_LIZARD_1, ENEMY_LIZARD_2, ENEMY_LIZARD_3,
    ENEMY_KNIGHT_1, ENEMY_KNIGHT_2, ENEMY_KNIGHT_3,
    ENEMY_GHOST_1, ENEMY_GHOST_2, ENEMY_GHOST_3,
    ENEMY_MUMMY_1, ENEMY_MUMMY_2, ENEMY_MUMMY_3,
    ENEMY_UNICORN_1, ENEMY_UNICORN_2, ENEMY_UNICORN_3,
    ENEMY_SKELETON_1, ENEMY_SKELETON_2, ENEMY_SKELETON_3,
    ENEMY_MIMIC_1, ENEMY_MIMIC_2, ENEMY_MIMIC_3,
    ENEMY_BIRD_1, ENEMY_BIRD_2, ENEMY_BIRD_3,
    ENEMY_SPINNER_1, ENEMY_SPINNER_2,
    ENEMY_QUAKE_1, ENEMY_QUAKE_2,
    ENEMY_DUKE,
    ENEMY_MIR,
    ENEMY_ZAK,
    ENEMY_MIRO_1, ENEMY_MIRO_2,
    ENEMY_IFRIT,
    ENEMY_NOLE, ENEMY_GOLA,
};

Entity::Entity(Attributes attrs) :
    _attrs  (attrs),
    _map    (nullptr)
{
    if(DEFAULT_PALETTES.count(_attrs.type_id))
        _attrs.palette = DEFAULT_PALETTES[_attrs.type_id];

    if(LIFTABLE_TYPES.count(_attrs.type_id))
        _attrs.liftable = true;

    if(FIGHTABLE_TYPES.count(_attrs.type_id))
        _attrs.fightable = true;

    if(ENEMY_TYPES.count(_attrs.type_id))
    {
        _attrs.fightable = true;
        _attrs.speed = 1;
        _attrs.behavior_id = 1;
    }
}

Entity::Entity(uint8_t type_id, uint8_t pos_x, uint8_t pos_y, uint8_t pos_z) :
    Entity({ 
        .type_id = type_id,
        .position = Position(pos_x, pos_y, pos_z)
    })
{}

Entity::Entity(const Entity& entity) :
    _map      (entity._map),
    _attrs    (entity._attrs)
{}

uint8_t Entity::entity_id() const 
{ 
    return _map->entity_id(this);
}

Json Entity::to_json(const World& world) const
{
    Json json;

    json["entityType"] = world.entity_type(_attrs.type_id)->name();

    json["posX"] = _attrs.position.x;
    json["posY"] = _attrs.position.y;
    json["posZ"] = _attrs.position.z;

    json["halfTileX"] = _attrs.position.half_x;
    json["halfTileY"] = _attrs.position.half_y;
    json["halfTileZ"] = _attrs.position.half_z;

    json["orientation"] = _attrs.orientation;
    json["palette"] = _attrs.palette;
    json["speed"] = _attrs.speed;

    json["fightable"] = _attrs.fightable;
    json["liftable"] = _attrs.liftable;
    json["canPassThrough"] = _attrs.can_pass_through;
    json["appearAfterPlayerMovedAway"] = _attrs.appear_after_player_moved_away;
    json["gravityImmune"] = _attrs.gravity_immune;

    json["talkable"] = _attrs.talkable;
    if(_attrs.talkable)
        json["dialogue"] = _attrs.dialogue;

    json["behaviorId"] = _attrs.behavior_id;
    if(this->has_persistence_flag())
        json["persistenceFlag"] = _attrs.persistence_flag.to_json();

    bool use_tiles_from_other_entity = (_attrs.entity_to_use_tiles_from != nullptr);
    json["useTilesFromOtherEntity"] = use_tiles_from_other_entity;
    if(use_tiles_from_other_entity)
        json["entityIdToUseTilesFrom"] = _attrs.entity_to_use_tiles_from->entity_id();

    json["flagUnknown_2_3"] = _attrs.flag_unknown_2_3;
    json["flagUnknown_2_4"] = _attrs.flag_unknown_2_4;
//    json["flagUnknown_3_5"] = _attrs.flag_unknown_3_5;

    if(!_attrs.mask_flags.empty())
    {
        json["maskFlags"] = Json::array();
        for(const EntityMaskFlag& mask : _attrs.mask_flags)
            json["maskFlags"].push_back(mask.to_json());
    }

    return json;
}

Entity* Entity::from_json(const Json& json, Map* map, const World& world)
{
    Attributes attrs {
        .type_id = world.entity_type((std::string)json.at("entityType"))->id(),

        .position = Position(
            json.at("posX"), json.at("posY"), json.at("posZ"),
            json.at("halfTileX"), json.at("halfTileY"), json.at("halfTileZ")
        ),

        .orientation = json.at("orientation"),
        .palette = json.at("palette"),
        .speed = json.at("speed"),

        .fightable = json.at("fightable"),
        .liftable = json.at("liftable"),
        .can_pass_through = json.at("canPassThrough"),
        .appear_after_player_moved_away = json.at("appearAfterPlayerMovedAway"),
        .gravity_immune = json.at("gravityImmune"),

        .talkable = json.at("talkable"),
        .dialogue = json.value<uint8_t>("dialogue", 0),

        .behavior_id = json.at("behaviorId"),

        .flag_unknown_2_3 = json.at("flagUnknown_2_3"),
        .flag_unknown_2_4 = json.at("flagUnknown_2_4"),
        .flag_unknown_3_5 = json.at("flagUnknown_3_5"),
    };

    bool use_tiles_from_other_entity = json.at("useTilesFromOtherEntity");
    if(use_tiles_from_other_entity)
    {
        uint8_t entity_id_to_use_tiles_from = json.at("entityIdToUseTilesFrom");
        attrs.entity_to_use_tiles_from = map->entity(entity_id_to_use_tiles_from);
    }

    if(json.contains("maskFlags"))
    {
        for(const Json& j : json.at("maskFlags"))
            attrs.mask_flags.push_back(EntityMaskFlag::from_json(j));
    }

    Entity* entity = new Entity(attrs);
    entity->_map = map;

    return entity;
}

Entity* Entity::from_rom(const md::ROM& rom, uint32_t addr, Map* map)
{
    Attributes attrs;

    // Byte 0
    uint8_t byte0 = rom.get_byte(addr);
    attrs.orientation = (byte0 & 0xC0) >> 6;
    attrs.position.x = byte0 & 0x3F;

    // Byte 1
    uint8_t byte1 = rom.get_byte(addr+1);
    attrs.palette = (byte1 & 0xC0) >> 6;
    attrs.position.y = byte1 & 0x3F;

    // Byte 2
    uint8_t byte2 = rom.get_byte(addr+2);
    attrs.fightable = byte2 & 0x80;
    attrs.liftable = byte2 & 0x40;
    attrs.talkable = byte2 & 0x20;
    attrs.flag_unknown_2_4 = byte2 & 0x10;
    attrs.flag_unknown_2_3 = byte2 & 0x08;
    attrs.speed = byte2 & 0x07;

    // Byte 3
    uint8_t byte3 = rom.get_byte(addr+3);
    attrs.position.half_x = byte3 & 0x80;
    attrs.position.half_y = byte3 & 0x40;
    attrs.flag_unknown_3_5 = byte3 & 0x20;
    
    bool use_tiles_from_other_entity = byte3 & 0x10;
    bool points_at_itself = false;
    if(use_tiles_from_other_entity)
    {
        uint8_t entity_id_to_use_tiles_from = byte3 & 0x0F;
        // There are a few occurences in the game where an entity points at itself on this property...
        if(entity_id_to_use_tiles_from == map->entities().size())
            points_at_itself = true;
        else
            attrs.entity_to_use_tiles_from = map->entity(entity_id_to_use_tiles_from);
    }

    // Byte 4
    uint8_t byte4 = rom.get_byte(addr+4);
    attrs.dialogue = (byte4 & 0xFC) >> 2;

    // Byte 5
    attrs.type_id = rom.get_byte(addr+5);

    // Byte 6
    uint8_t byte6 = rom.get_byte(addr+6);
    attrs.position.z = byte6 & 0x0F;
    attrs.can_pass_through = byte6 & 0x10;
    attrs.appear_after_player_moved_away = byte6 & 0x20;
    attrs.position.half_z = byte6 & 0x40;
    attrs.gravity_immune = byte6 & 0x80;

    // Byte 7
    uint8_t byte7 = rom.get_byte(addr+7);
    attrs.behavior_id = byte7;
    attrs.behavior_id |= (byte4 & 0x03) << 8;

    Entity* entity = new Entity(attrs);
    entity->_map = map;
    if(points_at_itself)
        entity->entity_to_use_tiles_from(entity);

    return entity;
}

std::vector<uint8_t> Entity::to_bytes() const
{
    // Byte 0
    uint8_t byte0 = _attrs.position.x & 0x3F;
    byte0 |= (_attrs.orientation & 0x3) << 6;

    // Byte 1
    uint8_t byte1 = _attrs.position.y & 0x3F;
    byte1 |= ((_attrs.palette & 0x3) << 6);

    // Byte 2
    uint8_t byte2 = _attrs.speed & 0x07;
    if(_attrs.fightable)          byte2 |= 0x80;
    if(_attrs.liftable)           byte2 |= 0x40;
    if(_attrs.talkable)           byte2 |= 0x20;
    if(_attrs.flag_unknown_2_4)   byte2 |= 0x10;
    if(_attrs.flag_unknown_2_3)   byte2 |= 0x08;

    // Byte 3
    bool use_tiles_from_other_entity = (_attrs.entity_to_use_tiles_from != nullptr);
    uint8_t entity_id_to_use_tiles_from = 0;
    if(_attrs.entity_to_use_tiles_from)
        entity_id_to_use_tiles_from = _attrs.entity_to_use_tiles_from->entity_id();

    uint8_t byte3 = entity_id_to_use_tiles_from & 0x0F;
    if(_attrs.position.half_x)                 byte3 |= 0x80;
    if(_attrs.position.half_y)                 byte3 |= 0x40;
    if(_attrs.flag_unknown_3_5)            byte3 |= 0x20;
    if(use_tiles_from_other_entity)  byte3 |= 0x10;

    // Byte 4
    uint8_t byte4 = (_attrs.dialogue & 0x3F) << 2;
    byte4 |= (_attrs.behavior_id >> 8) & 0x3;

    // Byte 5
    uint8_t byte5 = _attrs.type_id;

    // Byte 6
    uint8_t byte6 = _attrs.position.z & 0x0F;
    if(_attrs.can_pass_through)   byte6 |= 0x10;
    if(_attrs.appear_after_player_moved_away)   byte6 |= 0x20;
    if(_attrs.position.half_z)        byte6 |= 0x40;
    if(_attrs.gravity_immune)   byte6 |= 0x80;

    // Byte 7
    uint8_t byte7 = _attrs.behavior_id & 0x00FF;

    return { byte0, byte1, byte2, byte3, byte4, byte5, byte6, byte7 };
}
