#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <sstream>
#include <landstalker-lib/md_tools.hpp>
#include <landstalker-lib/tools/json.hpp>
#include <landstalker-lib/tools/flag.hpp>

class WorldTeleportTree {
private:
    std::string _name;
    uint16_t _map_id;
    uint16_t _paired_map_id = 0xFFFF;
    std::string _node_id;
    Flag _flag;

public:
    WorldTeleportTree(std::string name, uint16_t tree_map_id, std::string node_id, Flag flag) :
        _name       (std::move(name)),
        _map_id     (tree_map_id),
        _node_id    (std::move(node_id)),
        _flag       (std::move(flag))
    {}

    [[nodiscard]] const std::string& name() const { return _name; }

    [[nodiscard]] uint16_t map_id() const { return _map_id; }

    [[nodiscard]] uint16_t paired_map_id() const { return _paired_map_id; }
    void paired_map_id(uint16_t map_id) { _paired_map_id = map_id; }

    [[nodiscard]] const std::string& node_id() const { return _node_id; }
    [[nodiscard]] const Flag& flag() const { return _flag; }

    [[nodiscard]] Json to_json() const
    {
        Json json;
        json["name"] = _name;
        json["treeMapId"] = _map_id;
        json["nodeId"] = _node_id;

        std::stringstream ss;
        ss << std::hex << _flag.byte;
        json["flag"] = {
            { "byte", "0x" + ss.str() },
            { "bit", _flag.bit }
        };

        return json;
    }

    static WorldTeleportTree* from_json(const Json& json)
    {
        std::string name = json.at("name");
        uint16_t tree_map_id = json.at("treeMapId");
        std::string node_id = json.at("nodeId");

        uint16_t byte = 0;
        std::string byte_string = json.at("flag").at("byte");
        std::stringstream ss;
        ss << std::hex << byte_string.substr(2);
        ss >> byte;
        uint8_t bit = json.at("flag").at("bit");

        return new WorldTeleportTree(name, tree_map_id, node_id, Flag(byte, bit));
    }
};
