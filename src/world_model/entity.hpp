#pragma once

#include <utility>
#include <vector>
#include <md_tools.hpp>
#include <json.hpp>
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

    [[nodiscard]] Json to_json() const override
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
        return {visibility_if_flag_set, flag_byte, flag_bit};
    }
};

struct Position {

    Position() = default;

    Position(uint8_t ix, uint8_t iy, uint8_t iz, bool ihalf_x = false, bool ihalf_y = false, bool ihalf_z = false) :
        x(ix), y(iy), z(iz),
        half_x(ihalf_x), half_y(ihalf_y), half_z(ihalf_z)
    {}

    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t z = 0;

    bool half_x = false;
    bool half_y = false;
    bool half_z = false;
};

class Entity
{
public:
    struct Attributes {
        uint8_t type_id = 0;

        Position position = Position();

        uint8_t orientation = ENTITY_ORIENTATION_NE;
        uint8_t palette = 1;
        uint8_t speed = 0;

        bool fightable = false;
        bool liftable = false;
        bool can_pass_through = false;
        bool appear_after_player_moved_away = false;
        bool gravity_immune = false;

        bool talkable = false;
        uint8_t dialogue = 0;

        uint16_t behavior_id = 0;

        Entity* entity_to_use_tiles_from = nullptr;

        std::vector<EntityMaskFlag> mask_flags;
        Flag persistence_flag = Flag(0xFF, 0xFF);

        bool flag_unknown_2_3 = false;
        bool flag_unknown_2_4 = false;
        bool flag_unknown_3_5 = false;
    };

private:
    Attributes _attrs;
    Map* _map;

public:
    explicit Entity(Attributes attrs);
    Entity(uint8_t type_id, uint8_t pos_x, uint8_t pos_y, uint8_t pos_z);

    void map(Map* map) { _map = map; }
    [[nodiscard]] Map* map() const { return _map; }

    [[nodiscard]] uint8_t entity_id() const;

    [[nodiscard]] uint8_t entity_type_id() const { return _attrs.type_id; }
    void entity_type_id(uint8_t entity_type) { _attrs.type_id = entity_type; }

    [[nodiscard]] const Position& position() const { return _attrs.position; }
    Position& position() { return _attrs.position; }
    void position(const Position& pos) { _attrs.position = pos; }
    void position(uint8_t x, uint8_t y, uint8_t z) { _attrs.position.x = x; _attrs.position.y = y; _attrs.position.z = z; }

    [[nodiscard]] uint8_t orientation() const { return _attrs.orientation; }
    void orientation(uint8_t orientation) { _attrs.orientation = orientation; }

    [[nodiscard]] uint8_t palette() const { return _attrs.palette; }
    void palette(uint8_t palette) { _attrs.palette = palette; }

    [[nodiscard]] uint8_t speed() const { return _attrs.speed; }
    void speed(uint8_t speed) { _attrs.speed = speed; }

    [[nodiscard]] bool fightable() const { return _attrs.fightable; }
    void fightable(bool fightable) { _attrs.fightable = fightable; }

    [[nodiscard]] bool liftable() const { return _attrs.liftable; }
    void liftable(bool liftable) { _attrs.liftable = liftable; }

    [[nodiscard]] bool can_pass_through() const { return _attrs.can_pass_through; }
    void can_pass_through(bool can_pass_through) { _attrs.can_pass_through = can_pass_through; }

    [[nodiscard]] bool gravity_immune() const { return _attrs.gravity_immune; }
    void gravity_immune(bool value) { _attrs.gravity_immune = value; }

    [[nodiscard]] bool appear_after_player_moved_away() const { return _attrs.appear_after_player_moved_away; }
    void appear_after_player_moved_away(bool appear_after_player_moved_away) { _attrs.appear_after_player_moved_away = appear_after_player_moved_away; }

    [[nodiscard]] bool talkable() const { return _attrs.talkable; }
    void talkable(bool talkable) { _attrs.talkable = talkable; }

    [[nodiscard]] uint8_t dialogue() const { return _attrs.dialogue; }
    void dialogue(uint8_t dialogue) { _attrs.dialogue = dialogue; }

    [[nodiscard]] uint16_t behavior_id() const { return _attrs.behavior_id; }
    void behavior_id(uint16_t behavior_id) { _attrs.behavior_id = behavior_id; }
    
    [[nodiscard]] Entity* entity_to_use_tiles_from() const { return _attrs.entity_to_use_tiles_from; }
    void entity_to_use_tiles_from(Entity* entity) { _attrs.entity_to_use_tiles_from = entity; }

    [[nodiscard]] bool flag_unknown_2_3() const { return _attrs.flag_unknown_2_3; }
    void flag_unknown_2_3(bool value) { _attrs.flag_unknown_2_3 = value; }

    [[nodiscard]] bool flag_unknown_2_4() const { return _attrs.flag_unknown_2_4; }
    void flag_unknown_2_4(bool value) { _attrs.flag_unknown_2_4 = value; }

    [[nodiscard]] bool flag_unknown_3_5() const { return _attrs.flag_unknown_3_5; }
    void flag_unknown_3_5(bool value) { _attrs.flag_unknown_3_5 = value; }
    
    [[nodiscard]] const std::vector<EntityMaskFlag>& mask_flags() const { return _attrs.mask_flags; }
    std::vector<EntityMaskFlag>& mask_flags() { return _attrs.mask_flags; }
    void remove_when_flag_is_set(const Flag& flag) { _attrs.mask_flags.emplace_back(false, flag.byte & 0xFF, flag.bit); }
    void only_when_flag_is_set(const Flag& flag) { _attrs.mask_flags.emplace_back(true, flag.byte & 0xFF, flag.bit); }

    [[nodiscard]] bool has_persistence_flag() const { return _attrs.persistence_flag.byte != 0xFF && _attrs.persistence_flag.bit <= 7; }
    [[nodiscard]] Flag persistence_flag() const { return _attrs.persistence_flag; }
    void persistence_flag(Flag flag) { _attrs.persistence_flag = std::move(flag); }
    void clear_persistence_flag() { _attrs.persistence_flag = Flag(0xFF, 0xFF); }

    static Entity* from_rom(const md::ROM& rom, uint32_t addr, Map* map);
    [[nodiscard]] std::vector<uint8_t> to_bytes() const;

    [[nodiscard]] Json to_json(const World& world) const;
    static Entity* from_json(const Json& json, Map* map, const World& world);
};