#pragma once

#include <vector>

#include <landstalker-lib/tools/json.hpp>
#include <landstalker-lib/model/world.hpp>
#include <landstalker-lib/tools/vectools.hpp>
#include "logic_model/randomizer_world.hpp"

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
    const RandomizerWorld& _world;

    WorldNode* _start_node = nullptr;
    WorldNode* _end_node = nullptr;

    std::map<Item*, uint16_t> _forbidden_items;
    std::vector<WorldNode*> _forbidden_nodes_to_pick_items;

    std::vector<WorldNode*> _explored_nodes;
    std::vector<WorldNode*> _nodes_to_explore;

    std::vector<WorldPath*> _blocked_paths;
    std::vector<ItemSource*> _reachable_item_sources;

    std::vector<Item*> _relevant_items;
    
    std::vector<Item*> _starting_inventory;
    std::vector<Item*> _inventory;

    std::vector<std::pair<Item*, std::vector<ItemSource*>>> _scheduled_item_placements;

    uint32_t _step_count = 0;
    Json _debug_log;

public:
    explicit WorldSolver(const RandomizerWorld& world);

    void forbid_items(const std::map<Item*, uint16_t>& item_quantities);
    void forbid_item_type(Item* item);
    void forbid_taking_items_from_nodes(const std::vector<WorldNode*>& forbidden_nodes);

    void setup(WorldNode* start_node, WorldNode* end_node, const std::vector<Item*>& starting_inventory);
    bool try_to_solve(WorldNode* start_node, WorldNode* end_node, const std::vector<Item*>& starting_inventory);
    bool run_until_blocked();

    [[nodiscard]] const std::vector<Item*>& starting_inventory() const { return _starting_inventory; }
    void starting_inventory(const std::vector<Item*>& starting_inventory) { _starting_inventory = starting_inventory; }
    void update_current_inventory();

    [[nodiscard]] const std::vector<WorldPath*>& blocked_paths() const { return _blocked_paths; }
    [[nodiscard]] const std::vector<ItemSource*>& reachable_item_sources() const { return _reachable_item_sources; }
    [[nodiscard]] std::vector<ItemSource*> empty_reachable_item_sources() const;
    [[nodiscard]] const std::vector<Item*>& inventory() const { return _inventory; }

    [[nodiscard]] bool can_take_path(WorldPath* path) const;
    [[nodiscard]] std::vector<Item*> missing_items_to_take_path(WorldPath* path) const;
    [[nodiscard]] std::vector<WorldNode*> missing_nodes_to_take_path(WorldPath* path) const;

    [[nodiscard]] const std::vector<std::pair<Item*, std::vector<ItemSource*>>>& scheduled_item_placements() const { return _scheduled_item_placements; }

    [[nodiscard]] bool reached_end() const { return vectools::contains(_explored_nodes, _end_node); }
    [[nodiscard]] std::vector<Item*> find_minimal_inventory();

    [[nodiscard]] Json& debug_log() { return _debug_log; }
    [[nodiscard]] Json& debug_log_for_current_step() { return _debug_log["steps"][std::to_string(_step_count)]; }

private:
    void expand_exploration_zone();
    bool try_unlocking_paths();
    void take_path(WorldPath* path);
};
