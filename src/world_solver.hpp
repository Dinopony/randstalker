#pragma once

#include <vector>
#include "tools/unsorted_set.hpp"

class WorldRegion;
class ItemSource;
class Item;
class WorldPath;

class WorldSolver
{
private:
    WorldRegion* _start_node;
    WorldRegion* _end_node;
    UnsortedSet<Item*> _forbidden_items;
    UnsortedSet<WorldRegion*> _forbidden_regions_to_pick_items;

    std::vector<Item*> _inventory;

    UnsortedSet<WorldRegion*> _explored_regions;
    std::vector<WorldPath*> _paths_to_process;
    UnsortedSet<WorldPath*> _blocked_paths;
    UnsortedSet<ItemSource*> _picked_item_sources;

public:
    WorldSolver(WorldRegion* start_node, WorldRegion* end_node);

    void forbid_items(const UnsortedSet<Item*>& forbidden_items);
    void forbid_taking_items_from_regions(const UnsortedSet<WorldRegion*>& forbidden_regions);

    bool try_to_solve();

    std::vector<ItemSource*> reachable_item_sources(Item* item_constraint = nullptr);
    const std::vector<Item*>& inventory() const { return _inventory; }

private:
    void expand_exploration_zone();
    void explore_path(WorldPath* path);
    void pick_item_source(ItemSource* source);
};