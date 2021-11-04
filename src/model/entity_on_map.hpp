#pragma once

#include <vector>
#include "../tools/megadrive/rom.hpp"
#include "../extlibs/json.hpp"

class World;
class Entity;

class EntityOnMap
{
private:
    uint8_t _pos_x;
    uint8_t _pos_y;
    uint8_t _pos_z;
    Entity* _entity_type;
    uint8_t _palette;
    bool _horizontal_mirror;
    bool _flag_unknown_0_7;
    bool _fightable;
    bool _liftable;

    uint8_t _byte2_remainder;
    uint8_t _byte3;
    uint8_t _byte4;    
    uint8_t _byte6_msh;
    uint8_t _byte7;

    // AI behavior?
    // NPC textline?
    // Invisible?

public:
    EntityOnMap() {}

    Json to_json() const;
    static EntityOnMap* from_json(const Json& json, const World& world);

    static EntityOnMap* from_rom(const md::ROM& rom, uint32_t addr, const World& world);
    std::vector<uint8_t> to_bytes() const;
};