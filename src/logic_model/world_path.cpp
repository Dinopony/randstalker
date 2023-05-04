#include "world_path.hpp"
#include "world_node.hpp"
#include <landstalker-lib/tools/vectools.hpp>
#include <landstalker-lib/exceptions.hpp>
#include <utility>

WorldPath::WorldPath(WorldNode* from_node, WorldNode* to_node, uint16_t weight, std::vector<Item*> required_items,
                    std::vector<WorldNode*> required_nodes, std::vector<Item*> items_placed_when_crossing) :
    _from_node                  (from_node),
    _to_node                    (to_node),
    _weight                     (weight),
    _required_items             (std::move(required_items)),
    _required_nodes             (std::move(required_nodes)),
    _items_placed_when_crossing (std::move(items_placed_when_crossing))
{
    _from_node->add_outgoing_path(this);
    _to_node->add_ingoing_path(this);
}

std::vector<Item*> WorldPath::missing_items_to_cross(std::vector<Item*> player_inventory_copy, bool only_strictly_required_items)
{
    std::vector<Item*> items_to_test = _required_items;
    if(!only_strictly_required_items)
    {
        for(Item* item : _items_placed_when_crossing)
            items_to_test.emplace_back(item);
    }

    std::vector<Item*> missing_items;
    for (Item* item : items_to_test)
    {
        auto iter = std::find(player_inventory_copy.begin(), player_inventory_copy.end(), item);
        if (iter == player_inventory_copy.end())
        {
            missing_items.emplace_back(item);
        }
        else
        {
            // If item has been found, remove it from the inventory copy for it not to count several times
            // (case where quantity needed is > 1)
            player_inventory_copy.erase(iter);
        }
    }

    return missing_items;
}

bool WorldPath::has_explored_required_nodes(const std::vector<WorldNode*>& explored_nodes) const
{
    for(WorldNode* node : _required_nodes)
        if(!vectools::contains(explored_nodes, node))
            return false;
    return true;
}

bool WorldPath::is_perfect_opposite_of(WorldPath* other) const
{
    if(_from_node != other->_to_node)
        return false;
    if(_to_node != other->_from_node)
        return false;
    if(_weight != other->_weight)
        return false;
    if(_required_items != other->_required_items)
        return false;
    if(_required_nodes != other->_required_nodes)
        return false;
    if(_items_placed_when_crossing != other->_items_placed_when_crossing)
        return false;

    return true;
}

Json WorldPath::to_json(bool two_way) const
{
    Json json;

    json["fromId"] = _from_node->id();
    json["toId"] = _to_node->id();
    json["twoWay"] = two_way;

    if(_weight > 1)
        json["weight"] = _weight;

    if(!_required_items.empty())
    {
        json["requiredItems"] = Json::array();
        for(Item* item : _required_items)
            json["requiredItems"].emplace_back(item->name());
    }

    if(!_required_nodes.empty())
    {
        json["requiredNodes"] = Json::array();
        for(WorldNode* node : _required_nodes)
            json["requiredNodes"].emplace_back(node->id());
    }

    if(!_items_placed_when_crossing.empty())
    {
        for(Item* item : _items_placed_when_crossing)
            json["itemsPlacedWhenCrossing"].emplace_back(item->name());
    }

    return json;
}

static Item* find_item_from_name(const std::map<uint8_t, Item*>& items, const std::string& name)
{
    for(auto& [k, item] : items)
        if(item->name() == name)
            return item;

    std::stringstream msg;
    msg << "Could not find item with name '" << name << "' from required items in world paths JSON.";
    throw JsonParsingException(msg.str());
}

WorldPath* WorldPath::from_json(const Json& json, const std::map<std::string, WorldNode*>& nodes, const std::map<uint8_t, Item*>& items)
{
    const std::string& from_id = json.at("fromId");
    WorldNode* from_node = nodes.at(from_id);

    const std::string& to_id = json.at("toId");
    WorldNode* to_node = nodes.at(to_id);

    uint16_t weight = json.value("weight", 1);

    std::vector<Item*> required_items;
    if(json.contains("requiredItems"))
        for(const std::string& item_name : json.at("requiredItems"))
            required_items.emplace_back(find_item_from_name(items, item_name));

    std::vector<WorldNode*> required_nodes;
    if(json.contains("requiredNodes"))
        for(const std::string& node_id : json.at("requiredNodes"))
            required_nodes.emplace_back(nodes.at(node_id));

    std::vector<Item*> items_placed_when_crossing;
    if(json.contains("itemsPlacedWhenCrossing"))
        for(const std::string& item_name : json.at("itemsPlacedWhenCrossing"))
            items_placed_when_crossing.emplace_back(find_item_from_name(items, item_name));
        
    return new WorldPath(from_node, to_node, weight, required_items, required_nodes, items_placed_when_crossing);
}
