#pragma once

#include <vector>
#include "tools/unsorted_set.hpp"
#include "extlibs/json.hpp"
#include "world.hpp"

class WorldRegion;
class ItemSource;
class Item;
class WorldPath;

class WorldSolver
{
private:
    WorldRegion* _start_node;
    WorldRegion* _end_node;
    std::vector<Item*> _forbidden_item_instances;
    std::vector<Item*> _forbidden_item_types;
    UnsortedSet<WorldRegion*> _forbidden_regions_to_pick_items;

    UnsortedSet<WorldRegion*> _explored_regions;
    UnsortedSet<WorldRegion*> _regions_to_explore;
    
    UnsortedSet<WorldPath*> _blocked_paths;
    std::vector<ItemSource*> _reachable_item_sources;

    UnsortedSet<Item*> _relevant_items;
    
    std::vector<Item*> _starting_inventory;
    std::vector<Item*> _inventory;

    uint32_t _step_count;
    Json _debug_log;

public:
    WorldSolver()
    {}

    WorldSolver(WorldRegion* start_node, WorldRegion* end_node, const std::vector<Item*>& starting_inventory = {})
    {
        this->setup(start_node, end_node, starting_inventory);
    }

    WorldSolver(const World& world)
    {
        this->setup(world);
    }

    void setup(WorldRegion* start_node, WorldRegion* end_node, const std::vector<Item*>& starting_inventory = {});
    void setup(const World& world)
    { 
        this->setup(world.spawn_region(), world.end_region(), world.starting_inventory());
    }

    void forbid_item_instances(const std::vector<Item*>& forbidden_item_instances);
    void forbid_item_types(const std::vector<Item*>& forbidden_item_types);
    void forbid_taking_items_from_regions(const UnsortedSet<WorldRegion*>& forbidden_regions);

    bool try_to_solve();
    bool run_until_blocked();

    const std::vector<Item*>& starting_inventory() const { return _starting_inventory; }
    void starting_inventory(const std::vector<Item*>& starting_inventory) { _starting_inventory = starting_inventory; }
    void update_current_inventory();

    const UnsortedSet<WorldPath*>& blocked_paths() const { return _blocked_paths; }
    const std::vector<ItemSource*>& reachable_item_sources() const { return _reachable_item_sources; }
    std::vector<ItemSource*> empty_reachable_item_sources() const;
    const std::vector<Item*>& inventory() const { return _inventory; }

    bool can_take_path(WorldPath* path) const;
    std::vector<WorldRegion*> missing_regions_to_take_path(WorldPath* path) const;
    std::vector<Item*> missing_items_to_take_path(WorldPath* path) const;

    bool reached_end() const { return _explored_regions.contains(_end_node); }
    std::vector<Item*> find_minimal_inventory();

    Json& debug_log() { return _debug_log; }
    Json& debug_log_for_current_step() { return _debug_log["steps"][std::to_string(_step_count)]; }

private:
    void expand_exploration_zone();
    bool try_unlocking_paths();
};
