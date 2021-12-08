#include "world_logic.hpp"

#include "world_node.hpp"
#include "world_path.hpp"
#include "world_region.hpp"

#include <landstalker_lib/model/world_teleport_tree.hpp>
#include <landstalker_lib/model/world.hpp>
#include <landstalker_lib/exceptions.hpp>

#include "data/world_node.json.hxx"
#include "data/world_path.json.hxx"
#include "data/world_region.json.hxx"

#include <iostream>

WorldLogic::WorldLogic(const World& world)
{
    this->load_nodes(world);
    this->load_paths(world);
    this->load_regions();
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

void WorldLogic::load_nodes(const World& world)
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
            throw LandstalkerException("Could not find node '" + node_id + "' referenced by item source '" + source->name() + "'");
        }
    }

    std::cout << _nodes.size() << " nodes loaded." << std::endl;
}

void WorldLogic::load_paths(const World& world)
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
}

void WorldLogic::load_regions()
{
    Json regions_json = Json::parse(WORLD_REGIONS_JSON);
    for(const Json& region_json : regions_json)
        _regions.emplace_back(WorldRegion::from_json(region_json, _nodes));

    std::cout << _regions.size() << " regions loaded." << std::endl;

    for(auto& [id, node] : _nodes)
        if(node->region() == nullptr)
            throw LandstalkerException("Node '" + node->id() + "' doesn't belong to any region");
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

