#pragma once

#include "item.hpp"
#include "../tools/unsorted_set.hpp"

class WorldRegion;

class WorldPath
{
private:
    WorldRegion* _from_region;
    WorldRegion* _to_region;
    uint16_t _weight;
    std::vector<Item*> _required_items;
    std::vector<WorldRegion*> _required_regions;
    std::vector<Item*> _items_placed_when_crossing;
    
public:
    WorldPath(WorldRegion* from_region, 
                WorldRegion* to_region, 
                uint16_t weight = 1, 
                const std::vector<Item*>& required_items = {}, 
                const std::vector<WorldRegion*>& required_regions = {}, 
                const std::vector<Item*>& items_placed_when_crossing = {} );

    const std::vector<Item*>& required_items() const { return _required_items; }
    void add_required_item(Item* item) { _required_items.push_back(item); }

    WorldRegion* origin() const { return _from_region; }
    WorldRegion* destination() const { return _to_region; }

    const std::vector<WorldRegion*>& required_regions() const { return _required_regions; }
    bool has_explored_required_regions(const UnsortedSet<WorldRegion*>& explored_regions) const;

    const std::vector<Item*>& items_placed_when_crossing() const { return _items_placed_when_crossing; }
    void add_item_placed_when_crossing(Item* item) { _items_placed_when_crossing.push_back(item); }

    uint16_t weight() const { return _weight; }
    void weight(uint16_t weight) { _weight = weight; }

    std::vector<Item*> missing_items_to_cross(std::vector<Item*> playerInventoryCopy, bool only_strictly_required_items = false);

    bool is_perfect_opposite_of(WorldPath* other) const;

    Json to_json(bool two_way) const;
    static WorldPath* from_json(const Json& json, const std::map<std::string, WorldRegion*>& regions, const std::map<uint8_t, Item*>& items);  
};