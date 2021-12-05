#pragma once

#include <cstdint>
#include <string>
#include "../tools/megadrive/rom.hpp"

class WorldTeleportTree {
private:
    std::string _name;
    uint32_t _left_entrance_address;
    uint32_t _right_entrance_address;
    uint16_t _tree_map_id;
    std::string _node_id;

public:
    WorldTeleportTree(const std::string& name, uint32_t left_entrance_addr, uint32_t right_entrance_addr, uint16_t tree_map_id, const std::string& node_id) :
        _name                   (name),
        _left_entrance_address  (left_entrance_addr),
        _right_entrance_address (right_entrance_addr),
        _tree_map_id            (tree_map_id),
        _node_id                (node_id)
    {}

    const std::string& name() const { return _name; }

    uint32_t left_entrance_address() const { return _left_entrance_address; }
    uint32_t right_entrance_address() const { return _right_entrance_address; }

    uint16_t tree_map_id() const{ return _tree_map_id; }
    void tree_map_id(uint16_t map_id) { _tree_map_id = map_id; }

    const std::string& node_id() const { return _node_id; }

    Json to_json() const
    {
        Json json;
        json["name"] = _name;
        json["leftEntranceAddress"] = _left_entrance_address;
        json["rightEntranceAddress"] = _right_entrance_address;
        json["treeMapId"] = _tree_map_id;
        json["nodeId"] = _node_id;
        return json;
    }

    static WorldTeleportTree* from_json(const Json& json)
    {
        std::string name = json.at("name");
        uint32_t left_entrance_address = json.at("leftEntranceAddress");
        uint32_t right_entrance_address = json.at("rightEntranceAddress");
        uint16_t tree_map_id = json.at("treeMapId");
        std::string node_id = json.at("nodeId");

        return new WorldTeleportTree(name, left_entrance_address, right_entrance_address, tree_map_id, node_id);
    }
};
