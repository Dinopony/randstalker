#pragma once

#include <vector>
#include "../tools/megadrive/rom.hpp"
#include "../extlibs/json.hpp"

class World;
class Entity;

class EntityOnMap
{
private:
    Entity* _entity_type;

    uint8_t _pos_x;
    uint8_t _pos_y;
    uint8_t _pos_z;
    bool _half_tile_x;
    bool _half_tile_y;
    bool _half_tile_z;

    uint8_t _palette;
    
    bool _horizontal_mirror;
    bool _fightable;
    bool _liftable;
    bool _talkable;
    bool _can_pass_through;
    bool _appear_after_player_moved_away;

    bool _flag_unknown_0_7;
    bool _flag_unknown_6_7;

    /// ? bits 0x3F
    uint8_t _byte3_remainder;

    /// ?
    uint8_t _byte4;

    /// Has to do with static AI. For orcs, a value of 0 makes them unable to move as long as they have not been hit,
    /// and a value of 16 makes them perfectly frozen, except for some occasionnal random jumps (swamp shrine boss door guards behavior)
    uint8_t _byte2_remainder;

    /// Changing it for enemies change their default AI (0 makes an orc frozen at first, then moves normally once spotted)
    /// 106 => disappears when all enemies are killed in the room
    /// 113 => Mir wall barrier 1
    /// 114 => Mir wall barrier 2
    uint8_t _byte7; 

    // AI behavior?
    // NPC textline?
    // Invisible?

public:
    EntityOnMap() {}

    Json to_json() const;
    static EntityOnMap* from_json(const Json& json, const World& world);

    uint8_t pos_x() const { return _pos_x; }
    void pos_x(uint8_t pos_x) { _pos_x = pos_x; }

    uint8_t pos_y() const { return _pos_y; }
    void pos_y(uint8_t pos_y) { _pos_y = pos_y; }

    uint8_t pos_z() const { return _pos_z; }
    void pos_z(uint8_t pos_z) { _pos_z = pos_z; }

    bool half_tile_x() const { return _half_tile_x; }
    void half_tile_x(bool half_tile_x) { _half_tile_x = half_tile_x; }

    bool half_tile_y() const { return _half_tile_y; }
    void half_tile_y(bool half_tile_y) { _half_tile_y = half_tile_y; }

    bool half_tile_z() const { return _half_tile_z; }
    void half_tile_z(bool half_tile_z) { _half_tile_z = half_tile_z; }

    Entity* entity_type() const { return _entity_type; }
    void entity_type(Entity* entity_type) { _entity_type = entity_type; }

    uint8_t palette() const { return _palette; }
    void palette(uint8_t palette) { _palette = palette; }

    bool horizontal_mirror() const { return _horizontal_mirror; }
    void horizontal_mirror(bool horizontal_mirror) { _horizontal_mirror = horizontal_mirror; }

    bool flag_unknown_0_7() const { return _flag_unknown_0_7; }
    void flag_unknown_0_7(bool flag_unknown_0_7) { _flag_unknown_0_7 = flag_unknown_0_7; }

    bool fightable() const { return _fightable; }
    void fightable(bool fightable) { _fightable = fightable; }

    bool liftable() const { return _liftable; }
    void liftable(bool liftable) { _liftable = liftable; }

    uint8_t byte2_remainder() const { return _byte2_remainder; }
    void byte2_remainder(uint8_t byte2_remainder) { _byte2_remainder = byte2_remainder; }
    
    uint8_t byte3_remainder() const { return _byte3_remainder; }
    void byte3_remainder(uint8_t byte3_remainder) { _byte3_remainder = byte3_remainder; }
    
    uint8_t byte4() const { return _byte4; }
    void byte4(uint8_t byte4) { _byte4 = byte4; }
    
    bool can_pass_through() const { return _can_pass_through; }
    void can_pass_through(bool can_pass_through) { _can_pass_through = can_pass_through; }

    bool appear_after_player_moved_away() const { return _appear_after_player_moved_away; }
    void appear_after_player_moved_away(bool appear_after_player_moved_away) { _appear_after_player_moved_away = appear_after_player_moved_away; }

    bool flag_unknown_6_7() const { return _flag_unknown_6_7; }
    void flag_unknown_6_7(bool flag_unknown_6_7) { _flag_unknown_6_7 = flag_unknown_6_7; }
    
    uint8_t byte7() const { return _byte7; }
    void byte7(uint8_t byte7) { _byte7 = byte7; }

    static EntityOnMap* from_rom(const md::ROM& rom, uint32_t addr, const World& world);
    std::vector<uint8_t> to_bytes() const;
};