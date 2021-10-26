#pragma once

#include <cstdint>
#include <string>
#include "../md_tools/rom.hpp"

class WorldTeleportTree {
public:
    WorldTeleportTree(const std::string& name, uint32_t left_entrance_addr, uint32_t right_entrance_addr, uint16_t tree_map_id) :
        _name                   (name),
        _left_entrance_address  (left_entrance_addr),
        _right_entrance_address (right_entrance_addr),
        _tree_map_id            (tree_map_id)
    {}

    const std::string& name() const { return _name; }

    uint16_t tree_map_id() const{ return _tree_map_id; }
    void tree_map_id(uint16_t map_id) { _tree_map_id = map_id; }

    void write_to_rom(md::ROM& rom) const
    {
        rom.set_word(_left_entrance_address, _tree_map_id);
        rom.set_word(_right_entrance_address, _tree_map_id);
    }

    Json to_json() const
    {
        Json json;
        json["name"] = _name;
        json["leftEntranceAddress"] = _left_entrance_address;
        json["rightEntranceAddress"] = _right_entrance_address;
        json["treeMapId"] = _tree_map_id;
        return json;
    }

    static WorldTeleportTree* from_json(const Json& json)
    {
        std::string name = json.at("name");
        uint32_t left_entrance_address = json.at("leftEntranceAddress");
        uint32_t right_entrance_address = json.at("rightEntranceAddress");
        uint16_t tree_map_id = json.at("treeMapId");
        return new WorldTeleportTree(name, left_entrance_address, right_entrance_address, tree_map_id);
    }

private:
    std::string _name;
    uint32_t _left_entrance_address;
    uint32_t _right_entrance_address;
    uint16_t _tree_map_id;
};
