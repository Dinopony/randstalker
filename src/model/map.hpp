#pragma once

#include "../tools/megadrive/rom.hpp"
#include "../extlibs/json.hpp"

class EntityOnMap;
class World;

struct MapExit {
    uint8_t pos_x;
    uint8_t pos_y;

    uint16_t destination_map_id;
    uint8_t destination_x;
    uint8_t destination_y;

    /// 0x1 => Initially closed?
    /// 0x2 => NE
    /// 0x4 => NW
    /// 0x8 => ???
    /// 0x10 => NE Stairs
    /// 0x20 => NW Stairs
    uint8_t extra_byte;

    Json to_json() const {
        Json json;
        json["posX"] = pos_x;
        json["posY"] = pos_y;
        json["extraByte"] = extra_byte;
        json["destinationMapId"] = destination_map_id;
        json["destinationX"] = destination_x;
        json["destinationY"] = destination_y;
        return json;
    }
};

struct MapVariant {
    uint16_t map_variant_id;
    uint8_t flag_byte;
    uint8_t flag_bit;
    
    Json to_json() const {
        Json json;
        json["mapVariantId"] = map_variant_id;
        json["flagByte"] = flag_byte;
        json["flagBit"] = flag_bit;
        return json;
    }
};

class Map
{
private:
    uint32_t _id;
    uint32_t _address;

    uint8_t _tileset_id;
    uint8_t _primary_big_tileset_id;
    uint8_t _secondary_big_tileset_id;
    uint8_t _big_tileset_id;

    uint8_t _palette_id;
    uint8_t _room_height;
    uint8_t _background_music;

    uint8_t _unknown_param_1;
    uint8_t _unknown_param_2;

    uint8_t _base_chest_id;
    uint16_t _fall_destination;
    uint16_t _climb_destination;
    
    std::vector<EntityOnMap*> _entities;
    std::vector<MapExit> _exits;
    std::vector<MapVariant> _variants;

public:
    Map(uint16_t map_id, const md::ROM& rom, const World& world);
    ~Map();

    void clear();
    void write_to_rom(md::ROM& rom);

    uint16_t id() const { return _id; }

    uint16_t fall_destination() const { return _fall_destination; }
    uint16_t climb_destination() const { return _climb_destination; }

    uint8_t background_music() const { return _background_music; }
    void background_music(uint8_t music) { _background_music = music; }

    const std::vector<EntityOnMap*>& entities() const { return _entities; }
    EntityOnMap* entity(uint8_t entity_id) const { return _entities.at(entity_id); }

    const std::vector<MapExit>& exits() { return _exits; }
    const std::vector<MapVariant>& variants() { return _variants; }

    std::vector<uint8_t> entities_as_bytes() const;

    Json to_json(const World& world) const;
    Map* from_json(const Json& json);

private:
    void read_map_data(const md::ROM& rom);
    void write_map_data(md::ROM& rom);

    void read_base_chest_id(const md::ROM& rom);
    void write_base_chest_id(md::ROM& rom);

    void read_fall_destination(const md::ROM& rom);
    void write_fall_destination(md::ROM& rom);

    void read_climb_destination(const md::ROM& rom);
    void write_climb_destination(md::ROM& rom);

    void read_entities(const md::ROM& rom, const World& world);

    void read_exits(const md::ROM& rom);

    void read_variants(const md::ROM& rom);
};
