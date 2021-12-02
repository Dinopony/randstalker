#pragma once

#include "../tools/megadrive/rom.hpp"
#include "../extlibs/json.hpp"
#include "../tools/flag.h"

class Entity;
class World;
class MapPalette;

struct GlobalEntityMaskFlag : public Flag
{
    uint8_t first_entity_id;
    
    GlobalEntityMaskFlag(uint8_t byte, uint8_t bit, uint8_t first_entity_id) :
        Flag(byte, bit),
        first_entity_id(first_entity_id)
    {}

    GlobalEntityMaskFlag(Flag flag, uint8_t first_entity_id) :
        Flag(flag),
        first_entity_id(first_entity_id)
    {}

    uint16_t to_bytes() const
    {
        uint8_t msb = byte & 0xFF;
        uint8_t lsb = first_entity_id & 0x1F;
        lsb |= (bit & 0x7) << 5;

        return (((uint16_t)msb) << 8) + lsb;
    }

    Json to_json() const
    {
        Json json = Flag::to_json();
        json["firstEntityId"] = first_entity_id;
        return json;
    }
};

// RoomGfxSwapFlags
// byte_1 byte_2 = Map ID
// byte_3 --> flag_byte
// byte_4 & 0x7 --> flag_bit
// byte_4 & 0x78 --> tileswap id to look for in TileSwaps
    // 0x10 size
    // 0x0C map_id
    // 0x0E tileswap_id

class Map
{
private:
    uint16_t _id;
    uint32_t _address;

    uint8_t _tileset_id;
    uint8_t _primary_big_tileset_id;
    uint8_t _secondary_big_tileset_id;

    MapPalette* _palette;
    uint8_t _room_height;
    uint8_t _background_music;

    uint8_t _unknown_param_1;
    uint8_t _unknown_param_2;

    uint8_t _base_chest_id;
    uint16_t _fall_destination;
    uint16_t _climb_destination;
    
    std::vector<Entity*> _entities;
    
    std::map<Map*, Flag> _variants;
    Map* _parent_map;

    std::vector<uint16_t> _speaker_ids;

    std::vector<GlobalEntityMaskFlag> _global_entity_mask_flags;
    Flag _visited_flag;

public:
    Map(uint16_t map_id);
    Map(const Map& map);
    ~Map();
    
    void clear();

    void id(uint16_t id) { _id = id; }
    uint16_t id() const { return _id; }

    uint32_t address() const { return _address; }
    void address(uint32_t value) { _address = value; }

    bool is_variant() const { return _parent_map != nullptr; }

    uint8_t tileset_id() const { return _tileset_id; }
    void tileset_id(uint8_t tileset_id) { _tileset_id = tileset_id; }
    uint8_t primary_big_tileset_id() const { return _primary_big_tileset_id; }
    void primary_big_tileset_id(uint8_t value) { _primary_big_tileset_id = value; }
    uint8_t secondary_big_tileset_id() const { return _secondary_big_tileset_id; }
    void secondary_big_tileset_id(uint8_t value) { _secondary_big_tileset_id = value; }

    MapPalette* palette() const { return _palette; }
    void palette(MapPalette* palette) { _palette = palette; }
    uint8_t room_height() const { return _room_height; }
    void room_height(uint8_t value) { _room_height = value; }
    uint8_t background_music() const { return _background_music; }
    void background_music(uint8_t music) { _background_music = music; }

    uint8_t unknown_param_1() const { return _unknown_param_1; }
    void unknown_param_1(uint8_t value) { _unknown_param_1 = value; }
    uint8_t unknown_param_2() const { return _unknown_param_2; }
    void unknown_param_2(uint8_t value) { _unknown_param_2 = value; }

    uint8_t base_chest_id() const { return _base_chest_id; }
    void base_chest_id(uint8_t id) { _base_chest_id = id; }

    uint16_t fall_destination() const { return _fall_destination; }
    void fall_destination(uint16_t value) { _fall_destination = value; }
    uint16_t climb_destination() const { return _climb_destination; }
    void climb_destination(uint16_t value) { _climb_destination = value; }

    const std::vector<Entity*>& entities() const { return _entities; }
    const Entity& entity(uint8_t entity_id) const { return *_entities.at(entity_id); }
    Entity* entity(uint8_t entity_id) { return _entities.at(entity_id); }
    Entity* add_entity(Entity* entity);
    void insert_entity(uint8_t entity_id, Entity* entity);
    void remove_entity(uint8_t entity_id, bool delete_pointer = true);
    void move_entity(uint8_t entity_id, uint8_t entity_new_id);
    void clear_entities();
    uint8_t entity_id(const Entity* entity) const;

    const std::map<Map*, Flag>& variants() const { return _variants; }
    std::map<Map*, Flag>& variants() { return _variants; }
    const Flag& variant(Map* variant_map) const { return _variants.at(variant_map); }
    Flag& variant(Map* variant_map) { return _variants.at(variant_map); }
    void add_variant(Map* variant_map, uint8_t flag_byte, uint8_t flag_bit);

    const std::vector<uint16_t>& speaker_ids() const { return _speaker_ids; }
    std::vector<uint16_t>& speaker_ids() { return _speaker_ids; }

    const std::vector<GlobalEntityMaskFlag>& global_entity_mask_flags() const { return _global_entity_mask_flags; }
    std::vector<GlobalEntityMaskFlag>& global_entity_mask_flags() { return _global_entity_mask_flags; }

    const Flag& visited_flag() const { return _visited_flag; }
    void visited_flag(const Flag& flag) { _visited_flag = flag; }

    Json to_json(const World& world) const;
};
