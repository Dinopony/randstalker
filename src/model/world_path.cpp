#include "world_path.hpp"
#include "world_region.hpp"
#include "../exceptions.hpp"

WorldPath::WorldPath(WorldRegion* from_region, WorldRegion* to_region, uint16_t weight, const std::vector<Item*>& required_items, 
                    const std::vector<WorldRegion*>& required_regions, const std::vector<Item*>& items_placed_when_crossing) :
    _from_region                 (from_region),
    _to_region                   (to_region),
    _weight                      (weight),
    _required_items              (required_items),
    _required_regions            (required_regions),
    _items_placed_when_crossing  (items_placed_when_crossing)
{
    _from_region->add_outgoing_path(this);
    _to_region->add_ingoing_path(this);
}

std::vector<Item*> WorldPath::missing_items_to_cross(std::vector<Item*> player_inventory_copy, bool only_strictly_required_items)
{
    std::vector<Item*> items_to_test = _required_items;
    if(!only_strictly_required_items)
    {
        for(Item* item : _items_placed_when_crossing)
            items_to_test.push_back(item);
    }

    std::vector<Item*> missing_items;
    for (Item* item : items_to_test)
    {
        auto iter = std::find(player_inventory_copy.begin(), player_inventory_copy.end(), item);
        if (iter == player_inventory_copy.end())
        {
            missing_items.push_back(item);
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

bool WorldPath::has_explored_required_regions(const UnsortedSet<WorldRegion*>& explored_regions) const
{
    for(WorldRegion* region : _required_regions)
        if(!explored_regions.contains(region))
            return false;
    return true;
}

bool WorldPath::is_perfect_opposite_of(WorldPath* other) const
{
    if(_from_region != other->_to_region)
        return false;
    if(_to_region != other->_from_region)
        return false;
    if(_weight != other->_weight)
        return false;
    if(_required_items != other->_required_items)
        return false;
    if(_required_regions != other->_required_regions)
        return false;
    if(_items_placed_when_crossing != other->_items_placed_when_crossing)
        return false;

    return true;
}

Json WorldPath::to_json(bool two_way) const
{
    Json json;

    json["fromId"] = _from_region->id();
    json["toId"] = _to_region->id();
    json["twoWay"] = two_way;

    if(_weight > 1)
        json["weight"] = _weight;

    if(!_required_items.empty())
    {
        json["requiredItems"] = Json::array();
        for(Item* item : _required_items)
            json["requiredItems"].push_back(item->name());
    }

    if(!_required_regions.empty())
    {
        json["requiredRegions"] = Json::array();
        for(WorldRegion* region : _required_regions)
            json["requiredRegions"].push_back(region->id());
    }

    if(!_items_placed_when_crossing.empty())
    {
        for(Item* item : _items_placed_when_crossing)
            json["itemsPlacedWhenCrossing"].push_back(item->name());
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

WorldPath* WorldPath::from_json(const Json& json, const std::map<std::string, WorldRegion*>& regions, const std::map<uint8_t, Item*>& items)
{
    const std::string& from_id = json.at("fromId");
    WorldRegion* from_region = regions.at(from_id);

    const std::string& to_id = json.at("toId");
    WorldRegion* to_region = regions.at(to_id);

    uint16_t weight = json.value("weight", 1);

    std::vector<Item*> required_items;
    if(json.contains("requiredItems"))
        for(const std::string& item_name : json.at("requiredItems"))
            required_items.push_back(find_item_from_name(items, item_name));

    std::vector<WorldRegion*> required_regions;
    if(json.contains("requiredRegions"))
        for(const std::string& region_id : json.at("requiredRegions"))
            required_regions.push_back(regions.at(region_id));

    std::vector<Item*> items_placed_when_crossing;
    if(json.contains("itemsPlacedWhenCrossing"))
        for(const std::string& item_name : json.at("itemsPlacedWhenCrossing"))
            items_placed_when_crossing.push_back(find_item_from_name(items, item_name));
        
    return new WorldPath(from_region, to_region, weight, required_items, required_regions, items_placed_when_crossing);
}
