#pragma once

#include "../tools/megadrive/rom.hpp"
#include "../extlibs/json.hpp"

class EntityOnMap;
class World;

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

    std::vector<EntityOnMap*> _entities;

public:
    Map(uint16_t map_id, const md::ROM& rom, const World& world);
    ~Map();

    Json to_json() const;
    Map* from_json(const Json& json);
};