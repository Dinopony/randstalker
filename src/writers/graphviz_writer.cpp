#include "writers.hpp"

#include <vector>
#include <fstream>
#include "../tools/tools.hpp"

#include "../extlibs/json.hpp"

#include "../world_model/map.hpp"
#include "../world_model/map_connection.hpp"
#include "../world_model/world.hpp"

#include "../logic_model/data/world_path.json.hxx"
#include "../logic_model/world_node.hpp"
#include "../logic_model/world_logic.hpp"

namespace graphviz {

constexpr const char* COLORS[] = { "indianred2", "lightslateblue", "limegreen", "deeppink2", "darkorchid3", "chocolate2", "darkturquoise" };
size_t COLORS_SIZE = 7;

void output_logic_as_dot(const WorldLogic& logic, const std::string& path)
{
    std::ofstream graphviz(path);
    graphviz << "digraph {\n";

    graphviz << "\tgraph [pad=0.5, nodesep=0.4, ranksep=1];\n";
    graphviz << "\tnode[shape=rect];\n\n";

    Json paths_json = Json::parse(WORLD_PATHS_JSON);
    uint32_t path_i = 0;
    for(const Json& json : paths_json)
    {
        WorldNode* from = logic.node(json["fromId"]);
        WorldNode* to = logic.node(json["toId"]);
        graphviz << "\t" << from->id() << " -> " << to->id() << " [";
        if(json.contains("twoWay") && json.at("twoWay"))
            graphviz << "dir=both ";

        std::vector<std::string> required_names;
        if(json.contains("requiredItems"))
            for(const std::string& item_name : json.at("requiredItems"))
                required_names.push_back(item_name);
            
        if(json.contains("requiredNodes"))
            for(const std::string& node_id : json.at("requiredNodes"))
                required_names.push_back("Access to " + logic.node(node_id)->name());

        if(!required_names.empty())
        {
            const char* current_color = COLORS[path_i % COLORS_SIZE];
            graphviz << "color=" << current_color << " ";
            graphviz << "fontcolor=" << current_color << " ";
            graphviz << "label=\"" << tools::join(required_names, "\\n") << "\" ";
        }

        graphviz << "]\n";
        path_i++;
    }

    graphviz << "\n";
    for(auto& [id, node] : logic.nodes())
        graphviz << "\t" << id << " [label=\"" << node->name() << " [" << std::to_string(node->item_sources().size()) << "]\"]\n";

    graphviz << "}\n"; 
}

void output_maps_as_dot(const World& world, const std::string& path)
{
    std::ofstream graphviz(path);
    graphviz << "digraph {\n";

    graphviz << "\tgraph [pad=0.5, nodesep=0.4, ranksep=1];\n";
    graphviz << "\tnode[shape=rect];\n\n";
    
    for(auto& [map_id, map] : world.maps())
    {
        for(const MapConnection& connection : world.map_connections())
            graphviz << "\t" << connection.map_id_1() << " -> " << connection.map_id_2() << "\n";

        if(map->fall_destination() != 0xFFFF)
            graphviz << "\t" << map->id() << " -> " << map->fall_destination() << "\n";

        if(map->climb_destination() != 0xFFFF)
            graphviz << "\t" << map->id() << " -> " << map->climb_destination() << "\n";

        for(auto& [variant_map, flag] : map->variants())
        {
            graphviz << "\t" << map->id() << " -> " << variant_map->id() << "[color=indianred2, penwidth=2]\n";
            graphviz << "\t" << variant_map->id() << "[style=filled, fillcolor=indianred2]\n";            
        }
    }

    graphviz << "}\n"; 
}

} // namespace graphviz end