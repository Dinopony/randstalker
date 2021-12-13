#include "world_solver.hpp"

#include <landstalker_lib/model/item_source.hpp>
#include <landstalker_lib/exceptions.hpp>

#include "logic_model/world_node.hpp"

WorldSolver::WorldSolver(const WorldLogic& logic) : _logic (logic)
{}

/**
 * Setup a set of forbidden items in specified quantities that the solver will restrain itself from taking while trying
 * to solve the world.
 * @param item_quantities a map containing the amount of each item type that the solver will not take in its inventory
 */
void WorldSolver::forbid_items(const std::map<Item*, uint16_t>& item_quantities)
{
    for(auto& [item, quantity] : item_quantities)
        _forbidden_items[item] += quantity;
}

/**
 * Setup a forbidden item type that the solver will restrain itself from taking while trying to solve the world.
 * @param item an item type the solver will never take
 */
void WorldSolver::forbid_item_type(Item* item)
{
    _forbidden_items[item] = UINT16_MAX;
}

/**
 * Setup a set of forbidden WorldNodes where no item will be taken while trying to solve the world.
 * @param forbidden_nodes the nodes to forbid
 */
void WorldSolver::forbid_taking_items_from_nodes(const UnsortedSet<WorldNode*>& forbidden_nodes)
{ 
    _forbidden_nodes_to_pick_items = forbidden_nodes;
}

/**
 * Setup the solver, putting it back in its original state if it was already used.
 * @param start_node
 * @param end_node
 * @param starting_inventory
 */
void WorldSolver::setup(WorldNode* start_node, WorldNode* end_node, const std::vector<Item*>& starting_inventory)
{
    _start_node = start_node;
    _end_node = end_node;
    _nodes_to_explore = { _start_node };
    _starting_inventory = starting_inventory;

    _explored_nodes.clear();
    _blocked_paths.clear();
    _reachable_item_sources.clear();
    _relevant_items.clear();
    _starting_inventory.clear();
    _inventory.clear();
    _step_count = 0;
    
    _debug_log.clear();
}

/**
 * Try to reach end_node when starting at start_node and having the given starting items.
 * @param start_node the node where the solver will start
 * @param end_node the node the solver needs to reach
 * @param starting_inventory a vector containing items owned at start
 * @return true if end_node could be reached, false otherwise
 */
bool WorldSolver::try_to_solve(WorldNode* start_node, WorldNode* end_node, const std::vector<Item*>& starting_inventory)
{
    this->setup(start_node, end_node, starting_inventory);
    this->run_until_blocked();
    return this->reached_end();
}

/**
 * Run the solver until it is fully blocked.
 * @return true if something happened, false if the solver is in the exact same state as before running this function
 */
bool WorldSolver::run_until_blocked()
{
    _step_count++;
    _scheduled_item_placements.clear();
    size_t explored_nodes_count_at_start = _explored_nodes.size();
    size_t blocked_paths_count_at_start = _blocked_paths.size();

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

    bool new_nodes_explored = _explored_nodes.size() > explored_nodes_count_at_start;
    bool paths_unlocked = _blocked_paths.size() < blocked_paths_count_at_start;
    return new_nodes_explored || paths_unlocked;
}

/**
 * Update the solver inventory, by taking everything in reachable item sources (minus the forbidden items
 * defined on solver setup)
 */
void WorldSolver::update_current_inventory()
{
    _inventory = _starting_inventory;
    _inventory.reserve(_reachable_item_sources.size());

    std::map<Item*, uint16_t> forbidden_items_copy = _forbidden_items;

    for(ItemSource* source : _reachable_item_sources)
    {
        // If item is located in forbidden node, don't take it
        if(_forbidden_nodes_to_pick_items.contains(_logic.node(source->node_id())))
            continue;

        Item* item = source->item();
        if(!item)
            continue;

        // If item isn't useful for opening something, don't bother putting it in our inventory
        if(!_relevant_items.contains(item))
            continue;

        if(forbidden_items_copy.count(item) && forbidden_items_copy.at(item) > 0)
        {
            // If item is a forbidden item, don't take it
            forbidden_items_copy[item] -= 1;
            continue;
        }

        _inventory.emplace_back(item);
    }
}

/**
 * @return a list of all the empty item sources that can be currently reached by this solver
 */
std::vector<ItemSource*> WorldSolver::empty_reachable_item_sources() const
{
    std::vector<ItemSource*> empty_item_sources;
    empty_item_sources.reserve(_reachable_item_sources.size());

    for(ItemSource* source : _reachable_item_sources)
        if(source->empty())
            empty_item_sources.emplace_back(source);
    
    return empty_item_sources;
}

/**
 * Tests if all conditions (items, visited regions...) are met by the solver to cross the given WorldPath.
 * @param path the path to test
 * @return true if solver can take this path, false otherwise
 */
bool WorldSolver::can_take_path(WorldPath* path) const
{
    return this->missing_items_to_take_path(path).empty() && this->missing_nodes_to_take_path(path).empty();
}

/**
 * Get a list of the Items that are yet to be visited for the solver to be able to cross the given path.
 * @param path the path to test
 * @return the list of missing items
 */
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
            missing_items.emplace_back(item);
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

/**
 * Get a list of the WorldNodes that are yet to be visited for the solver to be able to cross the given path.
 * @param path the path to test
 * @return the list of missing nodes to take path
 */
std::vector<WorldNode*> WorldSolver::missing_nodes_to_take_path(WorldPath* path) const
{
    std::vector<WorldNode*> missing_nodes;
    const UnsortedSet<WorldNode*>& required_nodes = path->required_nodes();
    for(WorldNode* node : required_nodes)
    {
        // A required node was not explored yet, path cannot be taken
        if(!_explored_nodes.contains(node))
            missing_nodes.emplace_back(node);
    }

    return missing_nodes;
}

/**
 * Compute the minimal inventory, i.e. a minimal list of items which are strictly required to complete the seed.
 * If an item is in this list, it means the solver tried to solve the seed without it and didn't succeed.
 * It needs to be called on an already completed WorldSolver.
 * @return the minimal inventory required to beat the seed
 * @warning this function computes multiple sub-solves to get its returned value and therefore is rather expensive
 *          to call
 */
std::vector<Item*> WorldSolver::find_minimal_inventory()
{
    if(!this->reached_end())
    {
        std::ofstream dump_file("./crash_dump.json");
        dump_file << _debug_log.dump(4);
        dump_file.close();
        throw LandstalkerException("Tried to find minimal inventory on an incomplete WorldSolver");
    }

    std::vector<Item*> minimal_inventory;
    std::map<Item*, uint16_t> forbidden_items;

    for(Item* item : _inventory)
    {
        // Item is not a relevant item (e.g. an EkeEke in starting inventory), just ignore it
        if(!_relevant_items.contains(item))
            continue;

        std::map<Item*, uint16_t> forbidden_items_plus_one = forbidden_items;
        if(!forbidden_items_plus_one.count(item))
            forbidden_items_plus_one[item] = 0;
        forbidden_items_plus_one[item] += 1;

        WorldSolver solver(_logic);
        solver.forbid_items(forbidden_items_plus_one);
        if(solver.try_to_solve(_start_node, _end_node, _starting_inventory))
        {
            // Item can be freely removed: keep it removed for further solves
            forbidden_items = forbidden_items_plus_one;
        }
        else
        {
            // Item cannot be removed: it means it's required
            minimal_inventory.emplace_back(item);
        }
    }
 
    return minimal_inventory;
}


/////////////////////////////////////////////////////////////////////
///     INTERNAL FUNCTIONS
/////////////////////////////////////////////////////////////////////

/**
 * "Expand" the exploration zone by recursively taking all available paths that the solver is allowed to cross
 * given the items it currently owns and the regions it has already visited.
 * It updates most internal arrays inside the solver (blocked paths, explored nodes...).
 */
void WorldSolver::expand_exploration_zone()
{
    Json& debug_log = this->debug_log_for_current_step();
    if(!debug_log.contains("exploration"))
        debug_log["exploration"] = Json::array();

    while (!_nodes_to_explore.empty())
    {
        WorldNode* node = *_nodes_to_explore.begin();
        _nodes_to_explore.erase(node);
        _explored_nodes.insert(node);
        debug_log["exploration"].emplace_back("Explored " + node->id());

        // Process all outgoing paths
        for (WorldPath* path : node->outgoing_paths())
        {
            if(this->can_take_path(path))
            {
                this->take_path(path);
            }
            else
            {
                // If destination as already been explored or is already pending exploration, ignore it
                WorldNode* destination = path->destination();
                if (_explored_nodes.contains(destination) || _nodes_to_explore.contains(destination))
                    continue;

                debug_log["exploration"].emplace_back("Found blocked path to " + path->destination()->id());

                // For uncrossable blocked paths, add them to a pending list
                _blocked_paths.emplace_back(path);

                // Add required items to the list of relevant items encountered
                std::vector<Item*> required_items = path->required_items();
                for(Item* item : required_items)
                    _relevant_items.insert(item);
            }
        }

        // Add all item sources in this node to the reachable item sources list
        const std::vector<ItemSource*>& item_sources = node->item_sources();
        _reachable_item_sources.insert(_reachable_item_sources.end(), item_sources.begin(), item_sources.end());
    }
}

/**
 * Look at all the known blocked paths, and unlock any path that can now be unlocked thanks to new items or nodes
 * that would have been explored in the meantime.
 * @return true if at least one path was opened, false if no new path was opened
 */
bool WorldSolver::try_unlocking_paths()
{
    bool opened_at_least_one_path = false;

    Json& debug_log = this->debug_log_for_current_step();

    // Look for unlockable paths...
    for (size_t i=0 ; i < _blocked_paths.size() ; ++i)
    {
        WorldPath* path = _blocked_paths[i];
        if(this->can_take_path(path))
        {
            this->take_path(path);
            _blocked_paths.erase(path);
            --i;

            opened_at_least_one_path = true;
            debug_log["exploration"].emplace_back("Unlocked path " + path->origin()->id() + " --> " + path->destination()->id());
        }
    }

    return opened_at_least_one_path;
}

/**
 * Make the solver take a path, updating internal arrays and most notably the "scheduled item placements" array
 * which is populated using potential "itemsPlacedWhenCrossingPath" attached to the path.
 * @param path the path to take
 */
void WorldSolver::take_path(WorldPath* path)
{
    WorldNode* destination = path->destination();
    if (!_explored_nodes.contains(destination) && !_nodes_to_explore.contains(destination))
    {
        _nodes_to_explore.insert(destination);

        Json& debug_log = this->debug_log_for_current_step();
        debug_log["exploration"].emplace_back("Added " + destination->id() + " to accessible nodes");
    }

    for(Item* item : path->items_placed_when_crossing())
    {
        _scheduled_item_placements.emplace_back(std::make_pair(
                item, this->empty_reachable_item_sources()
        ));
    }
}
