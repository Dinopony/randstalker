#pragma once

#include <string>
#include <utility>
#include <landstalker_lib/tools/json.hpp>

#define ORIENTATION_NE 0x08
#define ORIENTATION_SE 0x48
#define ORIENTATION_SW 0x88
#define ORIENTATION_NW 0xC8

class SpawnLocation
{
private:
    std::string _id;
    uint16_t _map_id;
    uint8_t _pos_x;
    uint8_t _pos_y;
    uint8_t _orientation;
    std::string _node_id;
    uint8_t _starting_life;

public:
    SpawnLocation(std::string id, uint16_t map_id, uint8_t pos_x, uint8_t pos_y,
                    uint8_t orientation, uint8_t starting_life, std::string node_id = "") :
        _id             (std::move(id)),
        _map_id         (map_id),
        _pos_x          (pos_x),
        _pos_y          (pos_y),
        _orientation    (orientation),
        _starting_life  (starting_life),
        _node_id        (std::move(node_id))
    {}

    [[nodiscard]] const std::string& id() const { return _id; }
    [[nodiscard]] uint16_t map_id() const { return _map_id; }
    [[nodiscard]] uint8_t position_x() const { return _pos_x; }
    [[nodiscard]] uint8_t position_y() const { return _pos_y; }
    [[nodiscard]] uint8_t orientation() const { return _orientation; }
    [[nodiscard]] const std::string& node_id() const { return _node_id; }
    [[nodiscard]] uint8_t starting_life() const { return _starting_life; }

    [[nodiscard]] Json to_json() const
    {
        return Json {
            { "mapId", _map_id },
            { "posX", _pos_x },
            { "posY", _pos_y },
            { "orientation", _orientation },
            { "nodeId", _node_id },
            { "startingLife", _starting_life }
        };
    }
    
    static SpawnLocation* from_json(const std::string& id, const Json& json)
    {
        uint16_t map_id = json.at("mapId");
        uint8_t pos_x = json.at("posX");
        uint8_t pos_y = json.at("posY");
        uint8_t orientation = json.at("orientation");
        const std::string& node_id = json.at("nodeId");
        uint8_t starting_life = json.at("startingLife");

        return new SpawnLocation(id, map_id, pos_x, pos_y, orientation, starting_life, node_id);
    }

    void apply_json(const Json& json)
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
            _node_id = json.at("nodeId");
        if(json.contains("startingLife"))
            _starting_life = json.at("startingLife");        
    }
};
