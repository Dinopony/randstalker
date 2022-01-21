#include "io.hpp"

#include <landstalker_lib/io/io.hpp>
#include "landstalker_lib/tools/stringtools.hpp"

#include "../logic_model/world_region.hpp"
#include "../logic_model/hint_source.hpp"
#include "../logic_model/item_distribution.hpp"
#include "../logic_model/randomizer_world.hpp"

void ModelWriter::write_world_model(const RandomizerWorld& world)
{
    io::export_item_sources_as_json(world,"./json_data/item_source.json");
    io::export_items_as_json(world, "./json_data/item.json");
    io::export_entity_types_as_json(world, "./json_data/entity_type.json");
    io::export_maps_as_json(world, "./json_data/map.json");
    io::export_map_connections_as_json(world, "./json_data/map_connection.json");
    io::export_map_palettes_as_json(world, "./json_data/map_palette.json");
    io::export_teleport_trees_as_json(world, "./json_data/world_teleport_tree.json");
    io::export_game_strings_as_json(world, "./json_data/game_strings.json");
    io::export_blocksets_as_csv(world, "./json_data/blocksets/");
}

void ModelWriter::write_logic_model(const RandomizerWorld& world)
{
    Json nodes_json;
    for(auto& [id, node] : world.nodes())
        nodes_json[id] = node->to_json();
    dump_json_to_file(nodes_json, "./json_data/world_node.json");

    auto paths_copy = world.paths();
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
    dump_json_to_file(paths_json, "./json_data/world_path.json");

    Json regions_json = Json::array();
    for(WorldRegion* region : world.regions())
        regions_json.emplace_back(region->to_json());
    dump_json_to_file(regions_json, "./json_data/world_region.json");

    Json hints_json = Json::array();
    for(HintSource* hint_source : world.hint_sources())
        hints_json.emplace_back(hint_source->to_json());
    dump_json_to_file(hints_json, "./json_data/hint_source.json");

    Json spawns_json;
    for(auto& [id, spawn] : world.available_spawn_locations())
        spawns_json[id] = spawn->to_json();
    dump_json_to_file(spawns_json, "./json_data/spawn_location.json");

    Json distribs_json = Json::object();
    for(auto& [id, distrib] : world.item_distributions())
        distribs_json[std::to_string(id)] = distrib->to_json();
    dump_json_to_file(distribs_json, "./json_data/item_distribution.json");
}
