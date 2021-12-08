#pragma once

#include <vector>

#include <landstalker_lib/tools/unsorted_set.hpp>
#include <landstalker_lib/tools/json.hpp>
#include <landstalker_lib/model/world.hpp>

#include "logic_model/world_logic.hpp"

class WorldNode;
class ItemSource;
class Item;
class WorldPath;

/**
 * WorldSolver is a class used for checking a world solvability, or to explore a world
 * until the next blocking state to be able to fill it iteratively and make it solvable
 * in the end.
 * 
 * It works by setting a starting point and an end point, and optional constraints
 * (forbidding to take item X, to take items in node Y...) than can be used in some
 * circumstances to determine minimal requirements to solve the seed.
 */
class WorldSolver
{
private:
    const WorldLogic& _logic;

    WorldNode* _start_node;
    WorldNode* _end_node;
    std::vector<Item*> _forbidden_item_instances;
    std::vector<Item*> _forbidden_item_types;
    UnsortedSet<WorldNode*> _forbidden_nodes_to_pick_items;

    UnsortedSet<WorldNode*> _explored_nodes;
    UnsortedSet<WorldNode*> _nodes_to_explore;
    
    UnsortedSet<WorldPath*> _blocked_paths;
    std::vector<ItemSource*> _reachable_item_sources;

    UnsortedSet<Item*> _relevant_items;
    
    std::vector<Item*> _starting_inventory;
    std::vector<Item*> _inventory;

    uint32_t _step_count;
    Json _debug_log;

public:
    WorldSolver(const WorldLogic& logic);

    void forbid_item_instances(const std::vector<Item*>& forbidden_item_instances);
    void forbid_item_types(const std::vector<Item*>& forbidden_item_types);
    void forbid_taking_items_from_nodes(const UnsortedSet<WorldNode*>& forbidden_nodes);

    void setup(WorldNode* start_node, WorldNode* end_node, const std::vector<Item*>& starting_inventory);
    bool try_to_solve(WorldNode* start_node, WorldNode* end_node, const std::vector<Item*>& starting_inventory);
    bool run_until_blocked();

    const std::vector<Item*>& starting_inventory() const { return _starting_inventory; }
    void starting_inventory(const std::vector<Item*>& starting_inventory) { _starting_inventory = starting_inventory; }
    void update_current_inventory();

    const UnsortedSet<WorldPath*>& blocked_paths() const { return _blocked_paths; }
    const std::vector<ItemSource*>& reachable_item_sources() const { return _reachable_item_sources; }
    std::vector<ItemSource*> empty_reachable_item_sources() const;
    const std::vector<Item*>& inventory() const { return _inventory; }

    bool can_take_path(WorldPath* path) const;
    std::vector<WorldNode*> missing_nodes_to_take_path(WorldPath* path) const;
    std::vector<Item*> missing_items_to_take_path(WorldPath* path) const;

    bool reached_end() const { return _explored_nodes.contains(_end_node); }
    std::vector<Item*> find_minimal_inventory();

    Json& debug_log() { return _debug_log; }
    Json& debug_log_for_current_step() { return _debug_log["steps"][std::to_string(_step_count)]; }

private:
    void expand_exploration_zone();
    bool try_unlocking_paths();
};
