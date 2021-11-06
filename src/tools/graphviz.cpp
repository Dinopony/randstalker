#include "graphviz.hpp"

#include <vector>
#include <fstream>
#include "../model/world_path.json.hxx"
#include "../model/world_node.hpp"
#include "../model/map.hpp"
#include "../extlibs/json.hpp"
#include "../world.hpp"
#include "tools.hpp"

namespace graphviz {

constexpr char* COLORS[] = { "indianred2", "lightslateblue", "limegreen", "deeppink2", "darkorchid3", "chocolate2", "darkturquoise" };
size_t COLORS_SIZE = 7;

void output_logic_as_dot(const World& world, const std::string& path)
{
    std::ofstream graphviz(path);
    graphviz << "digraph {\n";

    graphviz << "\tgraph [pad=0.5, nodesep=0.4, ranksep=1];\n";
    graphviz << "\tnode[shape=rect];\n\n";

    Json paths_json = Json::parse(WORLD_PATHS_JSON);
    uint32_t path_i = 0;
    for(const Json& json : paths_json)
    {
        WorldNode* from = world.node(json["fromId"]);
        WorldNode* to = world.node(json["toId"]);
        graphviz << "\t" << from->id() << " -> " << to->id() << " [";
        if(json.contains("twoWay") && json.at("twoWay"))
            graphviz << "dir=both ";

        std::vector<std::string> required_names;
        if(json.contains("requiredItems"))
            for(const std::string& item_name : json.at("requiredItems"))
                required_names.push_back(item_name);
            
        if(json.contains("requiredNodes"))
            for(const std::string& node_id : json.at("requiredNodes"))
                required_names.push_back("Access to " + world.node(node_id)->name());

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
    for(auto& [id, node] : world.nodes())
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
        for(const MapExit& exit : map->exits())
            graphviz << "\t" << map->id() << " -> " << exit.destination_map_id << "\n";

        if(map->fall_destination() != 0xFFFF)
            graphviz << "\t" << map->id() << " -> " << map->fall_destination() << "\n";

        if(map->climb_destination() != 0xFFFF)
            graphviz << "\t" << map->id() << " -> " << map->climb_destination() << "\n";

        for(const MapVariant& variant : map->variants())
        {
            graphviz << "\t" << map->id() << " -> " << variant.map_variant_id << "[color=indianred2, penwidth=2]\n";
            graphviz << "\t" << variant.map_variant_id << "[style=filled, fillcolor=indianred2]\n";            
        }
    }

    graphviz << "}\n"; 
}

} // namespace graphviz end