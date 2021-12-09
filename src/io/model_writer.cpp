#include "io.hpp"

#include <landstalker_lib/model/item_source.hpp>
#include <landstalker_lib/model/entity_type.hpp>
#include <landstalker_lib/model/map.hpp>
#include <landstalker_lib/model/map_connection.hpp>
#include <landstalker_lib/model/map_palette.hpp>
#include <landstalker_lib/model/world_teleport_tree.hpp>
#include <landstalker_lib/model/world.hpp>
#include <landstalker_lib/tools/tools.hpp>
#include <landstalker_lib/tools/json.hpp>

#include "../logic_model/world_region.hpp"
#include "../logic_model/hint_source.hpp"
#include "../logic_model/item_distribution.hpp"
#include "../world_randomizer.hpp"

void ModelWriter::write_world_model(const World& world)
{
    Json item_sources_json = Json::array();
    for(ItemSource* source : world.item_sources())
        item_sources_json.emplace_back(source->to_json());
    tools::dump_json_to_file(item_sources_json, "./json_data/item_source.json");

    Json items_json;
    for(auto& [id, item] : world.items())
        items_json[std::to_string(id)] = item->to_json();
    tools::dump_json_to_file(items_json, "./json_data/item.json");

    Json entity_types_json = Json::object();
    for(auto& [id, entity_type] : world.entity_types())
        entity_types_json[std::to_string(id)] = entity_type->to_json();
    tools::dump_json_to_file(entity_types_json, "./json_data/entity_type.json");

    Json maps_json = Json::object();
    for(auto& [map_id, map] : world.maps())
        maps_json[std::to_string(map_id)] = map->to_json(world);
    tools::dump_json_to_file(maps_json, "./json_data/map.json");

    Json map_connections_json = Json::array();
    for(const MapConnection& connection : world.map_connections())
        map_connections_json.emplace_back(connection.to_json());
    tools::dump_json_to_file(map_connections_json, "./json_data/map_connection.json");

    Json map_palettes_json = Json::array();
    for(MapPalette* palette : world.map_palettes())
        map_palettes_json.emplace_back(palette->to_json());
    tools::dump_json_to_file(map_palettes_json, "./json_data/map_palette.json");

    Json trees_json = Json::array();
    for(auto& [tree_1, tree_2] : world.teleport_tree_pairs())
    {
        Json pair_json = Json::array();
        pair_json.emplace_back(tree_1->to_json());
        pair_json.emplace_back(tree_2->to_json());
        trees_json.emplace_back(pair_json);
    }
    tools::dump_json_to_file(trees_json, "./json_data/world_teleport_tree.json");

    Json strings_json;
    for(uint32_t i=0 ; i<world.game_strings().size() ; ++i)
    {
        std::stringstream hex_id;
        hex_id << "0x" << std::hex << i;
        strings_json[hex_id.str()] = world.game_strings()[i];
    }
    tools::dump_json_to_file(strings_json, "./json_data/game_strings.json");
}

void ModelWriter::write_logic_model(const WorldLogic& logic)
{
    Json nodes_json;
    for(auto& [id, node] : logic.nodes())
        nodes_json[id] = node->to_json();
    tools::dump_json_to_file(nodes_json, "./json_data/world_node.json");

    auto paths_copy = logic.paths();
    Json paths_json = Json::array();
    while(!paths_copy.empty())
    {
        auto it = paths_copy.begin();
        std::pair<WorldNode*, WorldNode*> node_pair = it->first;
        WorldPath* path = it->second;
        paths_copy.erase(it);

        bool two_way = false;
        std::pair<WorldNode*, WorldNode*> reverse_pair = std::make_pair(node_pair.second, node_pair.first);
        if(paths_copy.count(reverse_pair))
        {
            WorldPath* reverse_path = paths_copy.at(reverse_pair);
            if(path->is_perfect_opposite_of(reverse_path))
            {
                two_way = true;
                paths_copy.erase(reverse_pair);
            }
        }
        
        paths_json.emplace_back(path->to_json(two_way));
    }
    tools::dump_json_to_file(paths_json, "./json_data/world_path.json");

    Json regions_json = Json::array();
    for(WorldRegion* region : logic.regions())
        regions_json.emplace_back(region->to_json());
    tools::dump_json_to_file(regions_json, "./json_data/world_region.json");

    Json hints_json = Json::array();
    for(auto& [id, hint_source] : logic.hint_sources())
        hints_json.emplace_back(hint_source->to_json());
    tools::dump_json_to_file(hints_json, "./json_data/hint_source.json");

    Json spawns_json;
    for(auto& [id, spawn] : logic.spawn_locations())
        spawns_json[id] = spawn->to_json();
    tools::dump_json_to_file(spawns_json, "./json_data/spawn_location.json");

    Json distribs_json = Json::object();
    for(auto& [id, distrib] : logic.item_distributions())
        distribs_json[std::to_string(id)] = distrib->to_json();
    tools::dump_json_to_file(distribs_json, "./json_data/item_distribution.json");
}
