#pragma once

#include <vector>
#include "../tools/megadrive/rom.hpp"
#include "../extlibs/json.hpp"
#include "../tools/flag.h"

class World;
class EntityType;
class Map;

#define ENTITY_ORIENTATION_NE 0x0
#define ENTITY_ORIENTATION_SE 0x1
#define ENTITY_ORIENTATION_SW 0x2
#define ENTITY_ORIENTATION_NW 0x3

struct EntityMaskFlag : public Flag
{
    bool visibility_if_flag_set;

    EntityMaskFlag(bool p_visibility_if_flag_set, uint8_t flag_byte, uint8_t flag_bit) :
        Flag                    (flag_byte, flag_bit),
        visibility_if_flag_set  (p_visibility_if_flag_set)
    {}

    Json to_json() const
    {
        Json json = Flag::to_json();
        json["ifFlagSet"] = visibility_if_flag_set ? "show" : "hide";
        return json;
    }

    static EntityMaskFlag from_json(const Json& json)
    {
        bool visibility_if_flag_set = json.at("ifFlagSet");
        uint8_t flag_byte = json.at("flagByte");
        uint8_t flag_bit = json.at("flagBit");
        return EntityMaskFlag(visibility_if_flag_set, flag_byte, flag_bit);
    }
};

class EntityOnMap
{
private:
    Map* _map;
    uint8_t _entity_type_id;

    uint8_t _pos_x;
    uint8_t _pos_y;
    uint8_t _pos_z;
    bool _half_tile_x;
    bool _half_tile_y;
    bool _half_tile_z;

    uint8_t _orientation;
    uint8_t _palette;
    uint8_t _speed;

    bool _fightable;
    bool _liftable;
    bool _talkable;
    bool _can_pass_through;
    bool _appear_after_player_moved_away;

    uint8_t _dialogue;
    /// Changing it for enemies change their default AI (0 makes an orc frozen at first, then moves normally once spotted)
    /// 106 => disappears when all enemies are killed in the room
    /// 113 => Mir wall barrier 1
    /// 114 => Mir wall barrier 2
    uint16_t _behavior_id;

    EntityOnMap* _entity_to_use_tiles_from;

    bool _flag_unknown_2_3;
    bool _flag_unknown_2_4;
    bool _flag_unknown_3_5;
    bool _flag_unknown_6_7;

    std::vector<EntityMaskFlag> _mask_flags;

public:
    EntityOnMap();
    EntityOnMap(const EntityOnMap& entity);

    void map(Map* map) { _map = map; }
    Map* map() const { return _map; }

    uint8_t entity_id() const;

    uint8_t entity_type_id() const { return _entity_type_id; }
    void entity_type_id(uint8_t entity_type) { _entity_type_id = entity_type; }

    uint8_t pos_x() const { return _pos_x; }
    void pos_x(uint8_t pos_x) { _pos_x = pos_x; }

    uint8_t pos_y() const { return _pos_y; }
    void pos_y(uint8_t pos_y) { _pos_y = pos_y; }

    uint8_t pos_z() const { return _pos_z; }
    void pos_z(uint8_t pos_z) { _pos_z = pos_z; }

    void position(uint8_t x, uint8_t y, uint8_t z) { pos_x(x); pos_y(y); pos_z(z); }

    bool half_tile_x() const { return _half_tile_x; }
    void half_tile_x(bool half_tile_x) { _half_tile_x = half_tile_x; }

    bool half_tile_y() const { return _half_tile_y; }
    void half_tile_y(bool half_tile_y) { _half_tile_y = half_tile_y; }

    bool half_tile_z() const { return _half_tile_z; }
    void half_tile_z(bool half_tile_z) { _half_tile_z = half_tile_z; }

    uint8_t orientation() const { return _orientation; }
    void orientation(uint8_t orientation) { _orientation = orientation; }

    uint8_t palette() const { return _palette; }
    void palette(uint8_t palette) { _palette = palette; }

    uint8_t speed() const { return _speed; }
    void speed(uint8_t speed) { _speed = speed; }

    bool fightable() const { return _fightable; }
    void fightable(bool fightable) { _fightable = fightable; }

    bool liftable() const { return _liftable; }
    void liftable(bool liftable) { _liftable = liftable; }

    bool talkable() const { return _talkable; }
    void talkable(bool talkable) { _talkable = talkable; }

    bool can_pass_through() const { return _can_pass_through; }
    void can_pass_through(bool can_pass_through) { _can_pass_through = can_pass_through; }

    bool appear_after_player_moved_away() const { return _appear_after_player_moved_away; }
    void appear_after_player_moved_away(bool appear_after_player_moved_away) { _appear_after_player_moved_away = appear_after_player_moved_away; }

    uint8_t dialogue() const { return _dialogue; }
    void dialogue(uint8_t dialogue) { _dialogue = dialogue; }
    
    uint16_t behavior_id() const { return _behavior_id; }
    void behavior_id(uint16_t behavior_id) { _behavior_id = behavior_id; }
    
    EntityOnMap* entity_to_use_tiles_from() const { return _entity_to_use_tiles_from; }
    void entity_to_use_tiles_from(EntityOnMap* entity) { _entity_to_use_tiles_from = entity; }

    bool flag_unknown_2_3() const { return _flag_unknown_2_3; }
    void flag_unknown_2_3(bool value) { _flag_unknown_2_3 = value; }

    bool flag_unknown_2_4() const { return _flag_unknown_2_4; }
    void flag_unknown_2_4(bool value) { _flag_unknown_2_4 = value; }

    bool flag_unknown_3_5() const { return _flag_unknown_3_5; }
    void flag_unknown_3_5(bool value) { _flag_unknown_3_5 = value; }
    
    bool flag_unknown_6_7() const { return _flag_unknown_6_7; }
    void flag_unknown_6_7(bool value) { _flag_unknown_6_7 = value; }

    const std::vector<EntityMaskFlag>& mask_flags() const { return _mask_flags; }
    std::vector<EntityMaskFlag>& mask_flags() { return _mask_flags; }

    static EntityOnMap* from_rom(const md::ROM& rom, uint32_t addr, Map* map);
    std::vector<uint8_t> to_bytes() const;

    Json to_json(const World& world) const;
    static EntityOnMap* from_json(const Json& json, Map* map, const World& world);
};