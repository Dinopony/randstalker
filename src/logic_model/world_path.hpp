#pragma once

#include "../world_model/item.hpp"
#include "../tools/unsorted_set.hpp"

class WorldNode;

class WorldPath
{
private:
    WorldNode* _from_node;
    WorldNode* _to_node;
    uint16_t _weight;
    std::vector<Item*> _required_items;
    std::vector<WorldNode*> _required_nodes;
    std::vector<Item*> _items_placed_when_crossing;
    
public:
    WorldPath(WorldNode* from_node, 
                WorldNode* to_node, 
                uint16_t weight = 1, 
                const std::vector<Item*>& required_items = {}, 
                const std::vector<WorldNode*>& required_nodes = {}, 
                const std::vector<Item*>& items_placed_when_crossing = {} );

    const std::vector<Item*>& required_items() const { return _required_items; }
    std::vector<Item*>& required_items() { return _required_items; }
    void add_required_item(Item* item) { _required_items.push_back(item); }

    WorldNode* origin() const { return _from_node; }
    WorldNode* destination() const { return _to_node; }

    const std::vector<WorldNode*>& required_nodes() const { return _required_nodes; }
    bool has_explored_required_nodes(const UnsortedSet<WorldNode*>& explored_nodes) const;

    const std::vector<Item*>& items_placed_when_crossing() const { return _items_placed_when_crossing; }
    void add_item_placed_when_crossing(Item* item) { _items_placed_when_crossing.push_back(item); }

    uint16_t weight() const { return _weight; }
    void weight(uint16_t weight) { _weight = weight; }

    std::vector<Item*> missing_items_to_cross(std::vector<Item*> playerInventoryCopy, bool only_strictly_required_items = false);

    bool is_perfect_opposite_of(WorldPath* other) const;

    Json to_json(bool two_way) const;
    static WorldPath* from_json(const Json& json, const std::map<std::string, WorldNode*>& nodes, const std::map<uint8_t, Item*>& items);  
};