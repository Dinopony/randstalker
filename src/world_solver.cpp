#include "world_solver.hpp"

#include <fstream>

#include "model/world_node.hpp"
#include "model/item_source.hpp"

#include "exceptions.hpp"

void WorldSolver::forbid_item_instances(const std::vector<Item*>& forbidden_item_instances)
{ 
    _forbidden_item_instances = forbidden_item_instances;
}

void WorldSolver::forbid_item_types(const std::vector<Item*>& forbidden_item_types)
{
    _forbidden_item_types = forbidden_item_types;
}

void WorldSolver::forbid_taking_items_from_nodes(const UnsortedSet<WorldNode*>& forbidden_nodes)
{ 
    _forbidden_nodes_to_pick_items = forbidden_nodes;
}

void WorldSolver::setup(WorldNode* start_node, WorldNode* end_node, const std::vector<Item*>& starting_inventory)
{
    _start_node = start_node;
    _end_node = end_node;
    _nodes_to_explore = { start_node };

    if(!starting_inventory.empty())
        _starting_inventory = starting_inventory;

    _explored_nodes.clear(); 
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

bool WorldSolver::try_unlocking_paths()
{
    bool opened_at_least_one_path = false;

    Json& debug_log = this->debug_log_for_current_step();

    // Look for unlockable paths...
    for (size_t i=0 ; i < _blocked_paths.size() ; ++i)
    {
        WorldPath* path = _blocked_paths[i];

        if(!this->can_take_path(path))
            continue;

        // Unlock path by adding destination to nodes to explore (if it has not been explored yet)
        WorldNode* destination = path->destination();
        if (!_nodes_to_explore.contains(destination) && !_explored_nodes.contains(destination))
            _nodes_to_explore.insert(destination);

        // Remove path from blocked paths
        opened_at_least_one_path = true;
        _blocked_paths.erase(path);
        --i;

        debug_log["exploration"].push_back("Unlocked path " + path->origin()->id() + " --> " + path->destination()->id());
    }
    
    return opened_at_least_one_path;
}

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
        debug_log["exploration"].push_back("Explored " + node->id());

        // Process all outgoing paths
        for (WorldPath* path : node->outgoing_paths())
        {
            // If destination as already been explored or is already pending exploration, ignore it
            WorldNode* destination = path->destination();
            if (_explored_nodes.contains(destination) || _nodes_to_explore.contains(destination))
                continue;

            if(this->can_take_path(path))
            {
                // For crossable paths, add destination to the list of nodes to explore
                _nodes_to_explore.insert(destination);
                debug_log["exploration"].push_back("Added " + destination->id() + " to accessible nodes");
            }
            else
            {
                debug_log["exploration"].push_back("Found blocked path to " + path->destination()->id());

                // For uncrossable blocked paths, add them to a pending list
                _blocked_paths.push_back(path);

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

void WorldSolver::update_current_inventory()
{
    _inventory = _starting_inventory;
    _inventory.reserve(_reachable_item_sources.size());
    
    std::vector<Item*> forbidden_items_copy = _forbidden_item_instances;

    for(ItemSource* source : _reachable_item_sources)
    {
        // If item is located in forbidden node, don't take it
        if(_forbidden_nodes_to_pick_items.contains(source->node()))
            continue;

        Item* item = source->item();
        if(!item)
            continue;

        // If item isn't useful for opening something, don't bother putting it in our inventory
        if(!_relevant_items.contains(item))
            continue;

        // If item is contained in the forbidden item types, don't take it
        if(std::find(_forbidden_item_types.begin(), _forbidden_item_types.end(), item) != _forbidden_item_types.end())
            continue;

        auto it = std::find(forbidden_items_copy.begin(), forbidden_items_copy.end(), item);
        if(it != forbidden_items_copy.end())
        {
            // If item is a forbidden item, don't take it
            forbidden_items_copy.erase(it);
        }
        else
        {
            _inventory.push_back(item);
        }
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
    return this->missing_items_to_take_path(path).empty() && this->missing_nodes_to_take_path(path).empty();
}

std::vector<WorldNode*> WorldSolver::missing_nodes_to_take_path(WorldPath* path) const
{
    std::vector<WorldNode*> missing_nodes;
    const UnsortedSet<WorldNode*>& required_nodes = path->required_nodes();
    for(WorldNode* node : required_nodes)
    {
        // A required node was not explored yet, path cannot be taken
        if(!_explored_nodes.contains(node))
            missing_nodes.push_back(node);
    }

    return missing_nodes;
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
        if (!this->try_to_solve())
        {
            std::ofstream dump_file("./crash_dump.json");
            dump_file << _debug_log.dump(4);
            dump_file.close();
            throw RandomizerException("Tried to find minimal inventory on an uncompletable WorldSolver");
        }
    }

    std::vector<Item*> minimal_inventory;
    std::vector<Item*> forbidden_items;

    for(Item* item : _inventory)
    {
        // Item is not a relevant item (e.g. an EkeEke in starting inventory), just ignore it
        if(!_relevant_items.contains(item))
            continue;

        std::vector<Item*> forbidden_items_plus_one = forbidden_items;
        forbidden_items_plus_one.push_back(item);
        
        WorldSolver solver(_start_node, _end_node);
        solver.forbid_item_instances(forbidden_items_plus_one);
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
