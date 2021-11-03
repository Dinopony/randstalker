#include "world_region.hpp"
#include "world_node.hpp"

WorldRegion::WorldRegion(const std::string& name, const std::string& hint_name, 
                        const std::vector<WorldNode*> nodes, const std::vector<uint16_t>& dark_map_ids) :
    _name           (name),
    _hint_name      (hint_name),
    _nodes          (nodes),
    _dark_map_ids   (dark_map_ids)
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
        json["nodeIds"].push_back(node->id());

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
        nodes.push_back(all_nodes.at(node_id));

    std::vector<uint16_t> dark_map_ids;
    if(json.contains("darkMapIds")) 
        json.at("darkMapIds").get_to(dark_map_ids);

    return new WorldRegion(name, hint_name, nodes, dark_map_ids); 
}
