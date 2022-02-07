#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include "../../extlibs/landstalker_lib/md_tools.hpp"
#include "../../extlibs/landstalker_lib/tools/json.hpp"

class WorldTeleportTree {
private:
    std::string _name;
    uint16_t _tree_map_id;
    uint16_t _exterior_map_id;
    std::string _node_id;

public:
    WorldTeleportTree(std::string name, uint16_t tree_map_id, uint16_t exterior_map_id, std::string node_id) :
        _name                   (std::move(name)),
        _tree_map_id            (tree_map_id),
        _exterior_map_id        (exterior_map_id),
        _node_id                (std::move(node_id))
    {}

    [[nodiscard]] const std::string& name() const { return _name; }

    [[nodiscard]] uint16_t tree_map_id() const{ return _tree_map_id; }
    void tree_map_id(uint16_t map_id) { _tree_map_id = map_id; }

    [[nodiscard]] uint16_t exterior_map_id() const{ return _exterior_map_id; }

    [[nodiscard]] const std::string& node_id() const { return _node_id; }

    [[nodiscard]] Json to_json() const
    {
        Json json;
        json["name"] = _name;
        json["treeMapId"] = _tree_map_id;
        json["exteriorMapId"] = _exterior_map_id;
        json["nodeId"] = _node_id;
        return json;
    }

    static WorldTeleportTree* from_json(const Json& json)
    {
        std::string name = json.at("name");
        uint16_t tree_map_id = json.at("treeMapId");
        uint16_t exterior_map_id = json.at("exteriorMapId");
        std::string node_id = json.at("nodeId");

        return new WorldTeleportTree(name, tree_map_id, exterior_map_id, node_id);
    }
};
