#include "world_region.hpp"

#include <utility>
#include "world_node.hpp"
#include <landstalker_lib/model/item_source.hpp>

WorldRegion::WorldRegion(std::string name, std::string hint_name, std::vector<WorldNode*> nodes, std::vector<uint16_t> dark_map_ids) :
    _name           (std::move(name)),
    _hint_name      (std::move(hint_name)),
    _nodes          (std::move(nodes)),
    _dark_map_ids   (std::move(dark_map_ids))
{
    for(WorldNode* node : _nodes)
        node->region(this);
}

Json WorldRegion::to_json() const 
{
    Json json;

    json["name"] = _name;
    json["hintName"] = _hint_name;
    
    json["nodeIds"] = Json::array();
    for(WorldNode* node : _nodes)
        json["nodeIds"].emplace_back(node->id());

    if(!_dark_map_ids.empty())
        json["dark_map_ids"] = _dark_map_ids;

    return json;
}

WorldRegion* WorldRegion::from_json(const Json& json, const std::map<std::string, WorldNode*>& all_nodes)
{
    std::string name = json.at("name");
    std::string hint_name = json.value("hintName", "");

    std::vector<WorldNode*> nodes;
    for(std::string node_id : json.at("nodeIds"))
        nodes.emplace_back(all_nodes.at(node_id));

    std::vector<uint16_t> dark_map_ids;
    if(json.contains("darkMapIds")) 
        json.at("darkMapIds").get_to(dark_map_ids);

    return new WorldRegion(name, hint_name, nodes, dark_map_ids); 
}

std::map<std::string, ItemSource*> WorldRegion::item_sources() const
{
    std::map<std::string, ItemSource*> item_sources;
    for(WorldNode* node : _nodes)
    {
        for(ItemSource *source : node->item_sources())
            item_sources[source->name()] = source;
    }

    return item_sources;
}
