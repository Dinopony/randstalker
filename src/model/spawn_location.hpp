#pragma once

#include <string>
#include "../extlibs/json.hpp"
#include "world_node.hpp"

#define ORIENTATION_NE 0x08
#define ORIENTATION_SE 0x48
#define ORIENTATION_SW 0x88
#define ORIENTATION_NW 0xC8

class SpawnLocation
{
public:
    SpawnLocation(const std::string& id, uint16_t map_id, uint8_t pos_x, uint8_t pos_y, 
                    uint8_t orientation, WorldNode* node, uint8_t starting_life) :
        _id             (id),
        _map_id         (map_id),
        _pos_x          (pos_x),
        _pos_y          (pos_y),
        _orientation    (orientation),
        _node         (node),
        _starting_life  (starting_life)
    {}

    const std::string& id() const { return _id; }
    uint16_t map_id() const { return _map_id; }
    uint8_t position_x() const { return _pos_x; }
    uint8_t position_y() const { return _pos_y; }
    uint8_t orientation() const { return _orientation; }
    WorldNode* node() const { return _node; }
    uint8_t starting_life() const { return _starting_life; }

    Json to_json() const
    {
        Json json;
        json["mapID"] = _map_id;
        json["posX"] = _pos_x;
        json["posY"] = _pos_y;
        json["orientation"] = _orientation;
        json["nodeId"] = _node->id();
        json["startingLife"] = _starting_life;
        return json;
    }

    static SpawnLocation* from_json(const std::string& id, const Json& json, const std::map<std::string, WorldNode*>& nodes)
    {
        uint16_t map_id = json.at("mapId");
        uint8_t pos_x = json.at("posX");
        uint8_t pos_y = json.at("posY");
        uint8_t orientation = json.at("orientation");
        WorldNode* node = nodes.at(json.at("nodeId"));
        uint8_t starting_life = json.at("startingLife");

        return new SpawnLocation(id, map_id, pos_x, pos_y, orientation, node, starting_life);
    }

    void apply_json(const Json& json, const std::map<std::string, WorldNode*>& nodes)
    {
        if(json.contains("mapId"))
            _map_id = json.at("mapId");
        if(json.contains("posX"))
            _pos_x = json.at("posX");
        if(json.contains("posY"))
            _pos_y = json.at("posY");
        if(json.contains("orientation"))
            _orientation = json.at("orientation");
        if(json.contains("nodeId"))
            _node = nodes.at(json.at("nodeId"));
        if(json.contains("startingLife"))
            _starting_life = json.at("startingLife");        
    }

private:
    std::string _id;
    uint16_t _map_id;
    uint8_t _pos_x;
    uint8_t _pos_y;
    uint8_t _orientation;
    WorldNode* _node;
    uint8_t _starting_life;
};
