#pragma once

#include <string>
#include <vector>
#include <json.hpp>

class WorldNode;

class WorldRegion
{
private:
    std::string _name;
    std::string _hint_name;
    std::vector<WorldNode*> _nodes;
    std::vector<uint16_t> _dark_map_ids;

public:
    WorldRegion(const std::string& name, const std::string& hint_name, 
                const std::vector<WorldNode*> nodes, const std::vector<uint16_t>& dark_map_ids = {});

    const std::string& name() const { return _name; }
    const std::string& hint_name() const { return _hint_name; }
    const std::vector<WorldNode*>& nodes() const { return _nodes; }
    const std::vector<uint16_t>& dark_map_ids() const { return _dark_map_ids; }

    bool can_be_hinted() const { return !_hint_name.empty(); }

    Json to_json() const;
    static WorldRegion* from_json(const Json& json, const std::map<std::string, WorldNode*>& all_nodes);
};