#include "world_logic.hpp"

#include "world_node.hpp"
#include "world_path.hpp"
#include "world_region.hpp"

#include "../world_model/item_source.hpp"
#include "../world_model/item.hpp"
#include "../world_model/world_teleport_tree.hpp"
#include "../world_model/world.hpp"

#include "data/world_node.json.hxx"
#include "data/world_path.json.hxx"
#include "data/world_region.json.hxx"

#include "../randomizer_options.hpp"

#include <iostream>
#include "../exceptions.hpp"

WorldLogic::WorldLogic(const World& world, const RandomizerOptions& options)
{
    this->init_nodes(world);
    this->init_paths(world, options);
    this->init_regions();
}

WorldLogic::~WorldLogic()
{
    for (auto& [key, node] : _nodes)
        delete node;
    for (auto& [key, path] : _paths)
        delete path;
    for (WorldRegion* region : _regions)
        delete region;
}

void WorldLogic::init_nodes(const World& world)
{
    Json nodes_json = Json::parse(WORLD_NODES_JSON);
    for(auto& [node_id, node_json] : nodes_json.items())
    {
        WorldNode* new_node = WorldNode::from_json(node_id, node_json);
        _nodes[node_id] = new_node;
    }

    for(ItemSource* source : world.item_sources())
    {
        const std::string& node_id = source->node_id();
        try {
            _nodes.at(node_id)->add_item_source(source);
        } catch(std::out_of_range&) {
            throw RandomizerException("Could not find node '" + node_id + "' referenced by item source '" + source->name() + "'");
        }
    }

    std::cout << _nodes.size() << " nodes loaded." << std::endl;
}

void WorldLogic::init_paths(const World& world, const RandomizerOptions& options)
{
    Json paths_json = Json::parse(WORLD_PATHS_JSON);
    for(const Json& path_json : paths_json)
    {
        this->add_path(WorldPath::from_json(path_json, _nodes, world.items()));

        if(path_json.contains("twoWay") && path_json.at("twoWay"))
        {
            Json inverted_json = path_json;
            inverted_json["fromId"] = path_json.at("toId");
            inverted_json["toId"] = path_json.at("fromId");
            this->add_path(WorldPath::from_json(inverted_json, _nodes, world.items()));
        }
    }
    std::cout << _paths.size() << " paths loaded." << std::endl;   

    if(options.remove_gumi_boulder())
    {
        this->add_path(new WorldPath(_nodes.at("route_gumi_ryuma"), _nodes.at("gumi")));
    }

    // Handle paths related to specific tricks
    if(options.handle_ghost_jumping_in_logic())
    {
        this->add_path(new WorldPath(_nodes.at("route_lake_shrine"), _nodes.at("route_lake_shrine_cliff")));
    }

    // If damage boosting is taken in account in logic, remove all iron boots & fireproof requirements 
    if(options.handle_damage_boosting_in_logic())
    {
        for(auto& [pair, path] : _paths)
        {
            std::vector<Item*>& required_items = path->required_items();
            
            auto it = std::find(required_items.begin(), required_items.end(), world.item(ITEM_IRON_BOOTS));
            if(it != required_items.end())
                required_items.erase(it);
                
            it = std::find(required_items.begin(), required_items.end(), world.item(ITEM_FIREPROOF_BOOTS));
            if(it != required_items.end())
                required_items.erase(it);
        }
    }
    if(options.all_trees_visited_at_start())
    {
        std::vector<WorldNode*> required_nodes;
        if(!options.remove_tibor_requirement())
            required_nodes = { _nodes["tibor"] };

        for(auto& pair : world.teleport_tree_pairs())
        {
            WorldNode* first_node = this->node(pair.first->node_id());
            WorldNode* second_node = this->node(pair.second->node_id());
            this->add_path(new WorldPath(first_node, second_node, 1, {}, required_nodes));
            this->add_path(new WorldPath(second_node, first_node, 1, {}, required_nodes));
        }
    }
}

void WorldLogic::init_regions()
{
    Json regions_json = Json::parse(WORLD_REGIONS_JSON);
    for(const Json& region_json : regions_json)
        _regions.emplace_back(WorldRegion::from_json(region_json, _nodes));

    std::cout << _regions.size() << " regions loaded." << std::endl;

    for(auto& [id, node] : _nodes)
        if(node->region() == nullptr)
            throw RandomizerException("Node '" + node->id() + "' doesn't belong to any region");
}

WorldNode* WorldLogic::first_node_with_item(Item* item)
{
    for (auto& [key, node] : _nodes)
    {
        std::vector<ItemSource*> sources = node->item_sources();
        for (ItemSource* source : sources)
            if (source->item() == item)
                return node;
    }

    return nullptr;
}

WorldPath* WorldLogic::path(WorldNode* origin, WorldNode* destination)
{
    return _paths.at(std::make_pair(origin, destination));
}

WorldPath* WorldLogic::path(const std::string& origin_name, const std::string& destination_name)
{
    WorldNode* origin = _nodes.at(origin_name);
    WorldNode* destination = _nodes.at(destination_name);
    return this->path(origin, destination);
}

void WorldLogic::add_path(WorldPath* path)
{
    _paths[std::make_pair(path->origin(), path->destination())] = path;
}

WorldRegion* WorldLogic::region(const std::string& name) const
{
    for(WorldRegion* region : _regions)
        if(region->name() == name)
            return region;
    return nullptr;
}

