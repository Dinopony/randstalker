#include "world_solver.hpp"

#include "model/world_region.hpp"
#include "model/item_source.hpp"

#include "exceptions.hpp"

void WorldSolver::forbid_items(const UnsortedSet<Item*>& forbidden_items)
{ 
    _forbidden_items = forbidden_items;
}

void WorldSolver::forbid_taking_items_from_regions(const UnsortedSet<WorldRegion*>& forbidden_regions)
{ 
    _forbidden_regions_to_pick_items = forbidden_regions;
}

void WorldSolver::setup(WorldRegion* start_node, WorldRegion* end_node)
{
    _start_node = start_node;
    _end_node = end_node;
    _regions_to_explore = { start_node };

    _explored_regions.clear(); 
    _blocked_paths.clear();
    _reachable_item_sources.clear();
    _relevant_items.clear();
    _inventory.clear();
    _debug_log.clear();

    _step_count = 0;
}

bool WorldSolver::try_to_solve()
{
    this->run_until_blocked();
    return this->reached_end();
}

bool WorldSolver::run_until_blocked()
{
    _step_count++;
    size_t explored_regions_count_at_start = _explored_regions.size();

    bool unlocked_new_paths = true;
    while(unlocked_new_paths)
    {
        // 1) Expand the "exploration zone" by taking paths we can take with current inventory and exploration.
        this->expand_exploration_zone();

        // 2) Update the inventory by collecting all item sources reachable by the player
        this->update_current_inventory();

        // 3) While expanding the exploration zone, we encountered blocked paths. Find if we can open some of them.
        unlocked_new_paths = this->try_unlocking_paths();
    }

    return _explored_regions.size() > explored_regions_count_at_start;
}

bool WorldSolver::try_unlocking_paths()
{
    bool opened_at_least_one_path = false;

    Json& debug_log = this->debug_log_for_current_step();
    if(!debug_log.contains("unlockedPaths"))
        debug_log["unlockedPaths"] = Json::array();

    // Look for unlockable paths...
    for (size_t i=0 ; i < _blocked_paths.size() ; ++i)
    {
        WorldPath* path = _blocked_paths[i];

        if(!this->can_take_path(path))
            continue;

        // Unlock path by adding destination to regions to explore (if it has not been explored yet)
        WorldRegion* destination = path->destination();
        if (!_regions_to_explore.contains(destination) && !_explored_regions.contains(destination))
            _regions_to_explore.insert(destination);

        // Remove path from blocked paths
        opened_at_least_one_path = true;
        _blocked_paths.erase(path);
        --i;

        debug_log["unlockedPaths"].push_back(path->origin()->id() + " --> " + path->destination()->id());
    }
    
    return opened_at_least_one_path;
}

void WorldSolver::expand_exploration_zone()
{
    Json& debug_log = this->debug_log_for_current_step();
    if(!debug_log.contains("exploredRegions"))
        debug_log["exploredRegions"] = Json::array();

    while (!_regions_to_explore.empty())
    {
        WorldRegion* region = *_regions_to_explore.begin();
        _regions_to_explore.erase(region);
        _explored_regions.insert(region);
        debug_log["exploredRegions"].push_back(region->id());

        // Process all outgoing paths
        for (WorldPath* path : region->outgoing_paths())
        {
            // If destination as already been explored or is already pending exploration, ignore it
            WorldRegion* destination = path->destination();
            if (_explored_regions.contains(destination) || _regions_to_explore.contains(destination))
                continue;

            if(this->can_take_path(path))
            {
                // For crossable paths, add destination to the list of regions to explore
                _regions_to_explore.insert(destination);
            }
            else
            {
                // For uncrossable blocked paths, add them to a pending list
                _blocked_paths.push_back(path);

                // Add required items to the list of relevant items encountered
                std::vector<Item*> required_items = path->required_items();
                for(Item* item : required_items)
                    _relevant_items.insert(item);
            }
        }

        // Add all item sources in this region to the reachable item sources list
        const std::vector<ItemSource*>& item_sources = region->item_sources();
        _reachable_item_sources.insert(_reachable_item_sources.end(), item_sources.begin(), item_sources.end());
    }
}

void WorldSolver::update_current_inventory()
{
    _inventory.clear();
    _inventory.reserve(_reachable_item_sources.size());

    for(ItemSource* source : _reachable_item_sources)
    {
        // If item is located in forbidden region, don't take it
        if(_forbidden_regions_to_pick_items.contains(source->region()))
            continue;

        Item* item = source->item();
        if(!item)
            continue;

        // If item isn't useful for opening something, don't bother putting it in our inventory
        if(!_relevant_items.contains(item))
            continue;

        // If item is a forbidden item, don't take it
        if(!_forbidden_items.contains(item))
            _inventory.push_back(item);
    }
}

std::vector<ItemSource*> WorldSolver::empty_reachable_item_sources() const
{
    std::vector<ItemSource*> empty_item_sources;
    empty_item_sources.reserve(_reachable_item_sources.size());

    for(ItemSource* source : _reachable_item_sources)
        if(!source->item())
            empty_item_sources.push_back(source);
    
    return empty_item_sources;
}

bool WorldSolver::can_take_path(WorldPath* path) const
{
    return this->missing_items_to_take_path(path).empty() && this->missing_regions_to_take_path(path).empty();
}

std::vector<WorldRegion*> WorldSolver::missing_regions_to_take_path(WorldPath* path) const
{
    std::vector<WorldRegion*> missing_regions;
    const UnsortedSet<WorldRegion*>& required_regions = path->required_regions();
    for(WorldRegion* region : required_regions)
    {
        // A required region was not explored yet, path cannot be taken
        if(!_explored_regions.contains(region))
            missing_regions.push_back(region);
    }

    return missing_regions;
}

std::vector<Item*> WorldSolver::missing_items_to_take_path(WorldPath* path) const
{
    const UnsortedSet<Item*>& required_items = path->required_items();  
    std::vector<Item*> inventory_copy = _inventory;

    std::vector<Item*> missing_items;
    for (Item* item : required_items)
    {
        auto it = std::find(inventory_copy.begin(), inventory_copy.end(), item);

        // Item could not be found in inventory, path cannot be taken
        if (it == inventory_copy.end())
        {
            missing_items.push_back(item);
        }
        else
        {
            // Item has been found, remove it from the inventory copy for it not to count several times
            // (case where quantity needed is > 1)
            inventory_copy.erase(it);
        }
    }

    return missing_items;
}

std::vector<Item*> WorldSolver::find_minimal_inventory()
{
    if(!this->reached_end())
    {
        if(!this->try_to_solve())
            throw RandomizerException("Tried to find minimal inventory on an uncompletable WorldSolver");
    }

    std::vector<Item*> minimal_inventory;
    UnsortedSet<Item*> forbidden_items;

    for(Item* item : _inventory)
    {
        UnsortedSet<Item*> forbidden_items_plus_one = forbidden_items;
        forbidden_items_plus_one.insert(item);
        
        WorldSolver solver(_start_node, _end_node);
        solver.forbid_items(forbidden_items_plus_one);
        if(solver.try_to_solve())
        {
            // Item can be freely removed: keep it removed for further solves
            forbidden_items = forbidden_items_plus_one;
        }
        else
        {
            // Item cannot be removed: it means it's required
            minimal_inventory.push_back(item);
        }
    }
 
    return minimal_inventory;
}
