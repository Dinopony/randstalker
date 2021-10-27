#include "world_solver.hpp"

#include "model/world_region.hpp"
#include "model/item_source.hpp"

WorldSolver::WorldSolver(WorldRegion* start_node, WorldRegion* end_node) :
    _start_node         (start_node),
    _end_node           (end_node),
    _explored_regions   ({ start_node }),
    _paths_to_process   (start_node->outgoing_paths())
{}

void WorldSolver::forbid_items(const UnsortedSet<Item*>& forbidden_items)
{ 
    _forbidden_items = forbidden_items;
}

void WorldSolver::forbid_taking_items_from_regions(const UnsortedSet<WorldRegion*>& forbidden_regions)
{ 
    _forbidden_regions_to_pick_items = forbidden_regions;
}

bool WorldSolver::try_to_solve()
{
    while (true)
    {
        this->expand_exploration_zone();

        if (_explored_regions.contains(_end_node))
            return true;

        bool didnt_solve_any_path = true;
        for (WorldPath* blocked_path : _blocked_paths)
        {
            // Don't consider this path for exploration if we didn't explore all of the required regions
            if(!blocked_path->has_explored_required_regions(_explored_regions))
                continue;

            std::vector<Item*> missing_items = blocked_path->missing_items_to_cross(_inventory, true);
            std::vector<ItemSource*> item_sources_to_pick;
            bool all_located = true;
            for (Item *item : missing_items)
            {
                std::vector<ItemSource*> valid_reachable_sources;
                if (!_forbidden_items.contains(item))
                    valid_reachable_sources = this->reachable_item_sources(item);

                if (valid_reachable_sources.empty())
                {
                    all_located = false;
                    break;
                }

                item_sources_to_pick.push_back(*valid_reachable_sources.begin());
            }

            if (all_located)
            {
                // Path can be taken, initiate a sub-exploration by forking exploration state
                for (ItemSource *source : item_sources_to_pick)
                    this->pick_item_source(source);

                this->explore_path(blocked_path);
                didnt_solve_any_path = false;
                break;
            }
        }

        if (didnt_solve_any_path)
            return false;
    }
}

void WorldSolver::expand_exploration_zone()
{
    while (!_paths_to_process.empty())
    {
        WorldPath* first_path = *_paths_to_process.begin();
        _paths_to_process.erase(_paths_to_process.begin());

        // If we haven't explored all required regions yet, it means this path cannot be taken for now
        if(!first_path->has_explored_required_regions(_explored_regions))
        {
            _blocked_paths.push_back(first_path);
            continue;
        }

        // Region at the end of this path has already been explored, it's useless to evaluate it
        if (_explored_regions.contains(first_path->destination()))
            continue;

        std::vector<Item*> missing_items = first_path->missing_items_to_cross(_inventory, true);
        if (missing_items.empty())
        {
            // If we have already have the items to cross this path, explore it to expand our reach
            this->explore_path(first_path);
        }
        else
        {
            // Otherwise, add it to the list of blocked paths that will need to be processed later
            _blocked_paths.insert(first_path);
        }
    }
}

void WorldSolver::explore_path(WorldPath* path)
{
    // In case path was a blocking one, remove it from blocked paths
    _blocked_paths.erase(path);

    WorldRegion* destination = path->destination();
    _explored_regions.insert(destination);
    const std::vector<WorldPath*> outgoing_paths = destination->outgoing_paths();
    _paths_to_process.insert(_paths_to_process.end(), outgoing_paths.begin(), outgoing_paths.end());
}

void WorldSolver::pick_item_source(ItemSource *source)
{
    _picked_item_sources.insert(source);
    _inventory.push_back(source->item());
}

std::vector<ItemSource*> WorldSolver::reachable_item_sources(Item* item_constraint)
{
    std::vector<ItemSource*> reachable_sources;
    for (WorldRegion* region : _explored_regions)
    {
        // If picking items in this region is strictly forbidden, don't expose its item sources
        if (_forbidden_regions_to_pick_items.contains(region))
            continue;

        for (ItemSource* source : region->item_sources())
        {
            if (_picked_item_sources.contains(source))
                continue;

            if (item_constraint && source->item() != item_constraint)
                continue;

            reachable_sources.push_back(source);
        }
    }

    return reachable_sources;
}