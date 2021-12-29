#include "world_shuffler.hpp"

#include <landstalker_lib/constants/entity_type_codes.hpp>
#include <landstalker_lib/constants/item_codes.hpp>
#include <landstalker_lib/constants/values.hpp>
#include <landstalker_lib/tools/tools.hpp>
#include <landstalker_lib/tools/game_text.hpp>
#include <landstalker_lib/model/entity_type.hpp>
#include <landstalker_lib/model/item_source.hpp>
#include <landstalker_lib/model/world_teleport_tree.hpp>
#include <landstalker_lib/model/spawn_location.hpp>
#include <landstalker_lib/exceptions.hpp>

#include "logic_model/hint_source.hpp"
#include "logic_model/world_region.hpp"
#include "logic_model/item_distribution.hpp"
#include "world_solver.hpp"

#include <algorithm>
#include <iostream>

WorldShuffler::WorldShuffler(RandomizerWorld& world, const RandomizerOptions& options) :
    _world          (world),
    _solver         (world),
    _options        (options),
    _rng            (_options.seed())
{}

void WorldShuffler::randomize()
{
    // 1st pass: randomizations happening BEFORE randomizing items
    this->randomize_spawn_location();
    this->randomize_dark_rooms();
    
    if(_options.shuffle_tibor_trees())
        this->randomize_tibor_trees();

    this->randomize_fahl_enemies();

    // 2nd pass: randomizing items
    this->randomize_items();

    // 3rd pass: randomizations happening AFTER randomizing items
    this->randomize_hints();
    for(HintSource* hint_source : _world.hint_sources())
        hint_source->apply_text(_world);
}

///////////////////////////////////////////////////////////////////////////////////////
///        FIRST PASS RANDOMIZATIONS (before items)
///////////////////////////////////////////////////////////////////////////////////////

void WorldShuffler::randomize_spawn_location()
{
    if(!_world.spawn_location().empty())
        return;

    std::vector<std::string> spawn_location_pool = _options.possible_spawn_locations();
    if(spawn_location_pool.empty())
    {
        for(auto& [id, spawn] : _world.available_spawn_locations())
            spawn_location_pool.emplace_back(id);
    }

    tools::shuffle(spawn_location_pool, _rng);
    SpawnLocation* spawn = _world.available_spawn_locations().at(spawn_location_pool[0]);
    _world.spawn_location(*spawn);
}

void WorldShuffler::randomize_dark_rooms()
{
    // If dark region has already been set (e.g. through plando descriptor), no need to set it here
    if(_world.dark_region())
        return;

    std::vector<Item*> starting_inventory = _world.starting_inventory();
    Item* item_lantern = _world.item(ITEM_LANTERN);
    bool lantern_as_starting_item = std::find(starting_inventory.begin(), starting_inventory.end(), item_lantern) != starting_inventory.end();

    std::vector<WorldRegion*> possible_regions;
    for (WorldRegion* region : _world.regions())
    {
        // Don't allow spawning inside a dark node, unless we have lantern as starting item
        if(!lantern_as_starting_item && region == _world.spawn_node()->region())
            continue;

        if (!region->dark_map_ids().empty())
            possible_regions.emplace_back(region);
    }

    tools::shuffle(possible_regions, _rng);
    WorldRegion* dark_region = possible_regions[0];
    _world.dark_region(dark_region);
}

void WorldShuffler::randomize_tibor_trees()
{
    const std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>>& tree_pairs = _world.teleport_tree_pairs();
    
    std::vector<std::pair<uint16_t, uint16_t>> map_id_pairs;
    std::vector<WorldTeleportTree*> trees;
    for(auto& pair : tree_pairs)
    {
        map_id_pairs.emplace_back(std::make_pair(pair.first->tree_map_id(), pair.second->tree_map_id()));
        trees.emplace_back(pair.first);
        trees.emplace_back(pair.second);
    }

    tools::shuffle(trees, _rng);

    std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>> new_pairs;
    for(size_t i=0 ; i<trees.size() ; i+=2)
    {
        new_pairs.emplace_back(std::make_pair(trees[i], trees[i+1]));
        const std::pair<uint16_t, uint16_t>& map_id_pair = map_id_pairs[i/2];
        trees[i]->tree_map_id(map_id_pair.first);
        trees[i+1]->tree_map_id(map_id_pair.second);
    }

    _world.teleport_tree_pairs(new_pairs);
}

void WorldShuffler::randomize_fahl_enemies()
{
    if(!_world.fahl_enemies().empty())
        return;

    std::vector<uint8_t> easy_enemies = { 
        ENEMY_SLIME_2,      ENEMY_SLIME_3,      ENEMY_SLIME_4,      ENEMY_ORC_1,        ENEMY_MUMMY_1,
        ENEMY_ORC_2,        ENEMY_UNICORN_1,    ENEMY_MUSHROOM_1,   ENEMY_LIZARD_1,     ENEMY_WORM_1
    };
    std::vector<uint8_t> medium_enemies = { 
        ENEMY_SLIME_5,      ENEMY_SLIME_6,      ENEMY_ORC_3,        ENEMY_KNIGHT_1,     ENEMY_LIZARD_2,
        ENEMY_MIMIC_1,      ENEMY_MIMIC_2,      ENEMY_SKELETON_1,   ENEMY_UNICORN_2,    ENEMY_MUMMY_2, 
        ENEMY_MUMMY_3,      ENEMY_KNIGHT_2,     ENEMY_NINJA_1,      ENEMY_GIANT_1,      ENEMY_GIANT_2,
        ENEMY_WORM_2
    };
    std::vector<uint8_t> hard_enemies = { 
        ENEMY_MIMIC_3,      ENEMY_SKELETON_2,   ENEMY_SKELETON_3,   ENEMY_UNICORN_3,    ENEMY_UNICORN_3,
        ENEMY_KNIGHT_3,     ENEMY_NINJA_2,      ENEMY_NINJA_3,      ENEMY_GIANT_3,      ENEMY_QUAKE_1,
        ENEMY_LIZARD_3,     ENEMY_WORM_3
    };

    tools::shuffle(easy_enemies, _rng);
    tools::shuffle(medium_enemies, _rng);
    tools::shuffle(hard_enemies, _rng);

    _world.add_fahl_enemy(_world.entity_type(easy_enemies[0]));
    _world.add_fahl_enemy(_world.entity_type(easy_enemies[1]));
    _world.add_fahl_enemy(_world.entity_type(medium_enemies[0]));
    _world.add_fahl_enemy(_world.entity_type(medium_enemies[1]));
    _world.add_fahl_enemy(_world.entity_type(hard_enemies[0]));
}


///////////////////////////////////////////////////////////////////////////////////////
///        SECOND PASS RANDOMIZATIONS (items)
///////////////////////////////////////////////////////////////////////////////////////

void WorldShuffler::randomize_items()
{
    this->init_item_pool();

    _solver.setup(_world.spawn_node(), _world.end_node(), _world.starting_inventory());

    bool explored_new_nodes = true;
    while(explored_new_nodes)
    {
        // Run a solver step to reach a "blocked" state where something needs to be placed in order to continue
        explored_new_nodes = _solver.run_until_blocked();

        // Place all "scheduled" item placements, which correspond to "itemPlacedWhenCrossing" attributes of WorldPaths
        // solver encountered during last step
        for(auto& [item, item_sources] : _solver.scheduled_item_placements())
            this->place_item_randomly(item, item_sources);

        // Place one or several key items in order to open a blocked path
        this->open_random_blocked_path();

        // Item sources changed, force the solver to update its inventory
        _solver.update_current_inventory();
    }

    // Place the remaining items from the item pool in the remaining sources
    this->place_remaining_items();

    // Analyse items required to complete the seed
    Json& debug_log = _solver.debug_log();
    _minimal_items_to_complete = _solver.find_minimal_inventory();
    debug_log["requiredItems"] = Json::array();
    for (Item* item : _minimal_items_to_complete)
        debug_log["requiredItems"].emplace_back(item->name());
}

/**
 * Initialize the _item_pool internal vector which contains all items that need to be placed during process.
 * The item pool must have the exact size of the empty item sources that need to be filled in order to have a
 * consistent 1:1 matching between the two of them.
 * The _item_pool is built from the required ItemDistribution, and already placed items (in case of plandos) are
 * subtracted from the pool.
 */
void WorldShuffler::init_item_pool()
{
    _item_pool.clear();

    // TODO: Improve this behavior by using the item pool as a list of how to fill remaining item sources
    //       instead of what is meant to be inside all item sources.
    size_t filled_item_sources_count = 0;
    for(ItemSource* source : _world.item_sources())
        if(!source->empty())
            filled_item_sources_count++;
    if(filled_item_sources_count == _world.item_sources().size())
        return;

    _item_pool.reserve(_world.item_sources().size());
    _item_pool_quantities = _world.item_quantities();

    // Count quantities already in place
    for(ItemSource* source : _world.item_sources())
    {
        if(source->empty())
            continue;

        uint8_t item_id = source->item()->id();
        if(_item_pool_quantities[item_id] == 0)
        {
            throw LandstalkerException("There are more " + source->item()->name() +
                                       " already placed than the expected number in the item pool");
        }
        _item_pool_quantities[item_id] -= 1;
    }

    // Build the item pool from the quantities read inside the ItemDistribution objects, and shuffle it
    for(auto& [item_id, quantity] : _item_pool_quantities)
    {
        for(uint16_t i=0 ; i<quantity ; ++i)
        {
            if(item_id >= ITEM_GOLDS_START)
                _item_pool.emplace_back(this->generate_gold_item());
            else
                _item_pool.emplace_back(_world.item(item_id));
        }
    }
    tools::shuffle(_item_pool, _rng);

    // Count the empty item sources, and compare this count to the item pool size to handle invalid cases
    size_t empty_item_sources_count = 0;
    for(ItemSource* source : _world.item_sources())
    {
        if(source->empty())
            empty_item_sources_count += 1;
    }

    if(_item_pool.size() > empty_item_sources_count)
    {
        throw LandstalkerException("The number of items in item pool is not the same as the number of item sources (" +
                                    std::to_string(_item_pool.size()) + " =/= " +
                                    std::to_string(_world.item_sources().size()) + ")");
    }
    else if(_item_pool.size() < empty_item_sources_count)
    {
        size_t missing_item_count = _world.item_sources().size() - _item_pool.size();
        std::cout << "Warning: Item pool (" << _item_pool.size() << " items) is smaller than the item sources pool ("
                                            << _world.item_sources().size() << " item sources)."
                                            << "Remaining sources will remain empty.\n\n";

        for(size_t i=0 ; i<missing_item_count ; ++i)
            _item_pool.emplace_back(_world.item(ITEM_NONE));
    }
}

/**
 * Generate an ItemGolds that has a randomized gold value, and add it to the World.
 * @return the new item
 */
Item* WorldShuffler::generate_gold_item()
{
    std::normal_distribution<double> distribution(40.0, 18.0);
    double gold_value = distribution(_rng);

    if (gold_value < 1)
        gold_value = 1;
    else if (gold_value > 255)
        gold_value = 255;

    return _world.add_gold_item(static_cast<uint8_t>(gold_value));
}

/**
 * Place the given item randomly in one of the given item sources
 * @param item the item to place
 * @param possible_sources the item sources that are eligible for item placement
 * @return the item source where the item was placed
 * @throw LandstalkerException if item could not be placed in any of the possible item sources
 * @throw LandstalkerException if item could not be found inside item pool
 */
ItemSource* WorldShuffler::place_item_randomly(Item* item, std::vector<ItemSource*> possible_sources)
{
    if(_item_pool_quantities[item->id()] == 0)
    {
        std::cout << "Ignored placement of " << item->name() << " after crossing path because there are no more"
                                                                " instances of it inside the item pool." << std::endl;
        return nullptr;
    }

    tools::shuffle(possible_sources, _rng);

    ItemSource* picked_item_source = nullptr;
    for(ItemSource* source : possible_sources)
    {
        if(source->empty() && test_item_source_compatibility(source, item))
        {
            source->item(item);
            picked_item_source = source;
            break;
        }
    }

    if(!picked_item_source)
    {
        throw LandstalkerException("Could not place " + item->name() + " in any of the "
                                   + std::to_string(possible_sources.size()) + " possible sources.");
    }

    _item_pool.erase(std::find(_item_pool.begin(), _item_pool.end(), item));
    _item_pool_quantities[item->id()] -= 1;
    return picked_item_source;
}

/**
 * Fill the given item source with a random compatible item from the item pool.
 * If source could not be filled with anything, a warning message will be outputted.
 * @param source the ItemSource to fill
 * @return the Item that was used to fill the ItemSource
 */
Item* WorldShuffler::fill_item_source_randomly(ItemSource* source)
{
    for(auto it=_item_pool.begin() ; it!=_item_pool.end() ; ++it)
    {
        Item* item = *it;
        if(test_item_source_compatibility(source, item))
        {
            source->item(item);
            _item_pool.erase(it);
            _item_pool_quantities[item->id()] -= 1;
            return item;
        }
    }

    std::cout << "[WARNING] Item source '" << source->name() << "' could not be filled with any item of the item pool.\n";
    return nullptr;
}

/**
 * Checks if the given Item can be placed inside the given ItemSource if we follow strictly the ItemDistribution rules.
 * @param source the ItemSource to test
 * @param item the Item to test
 * @return true if the Item can be placed inside the ItemSource, false otherwise
 */
bool WorldShuffler::test_item_source_compatibility(ItemSource* source, Item* item) const
{
    if(source->is_chest() || source->is_npc_reward())
        return true;

    ItemSourceOnGround* cast_source = reinterpret_cast<ItemSourceOnGround*>(source);
    if(item->id() >= ITEM_GOLDS_START)
        return false;
    if(!cast_source->can_be_taken_only_once() && !_world.item_distribution(item->id())->allowed_on_ground())
        return false;

    if(source->is_shop_item())
    {
        if(item->id() == ITEM_NONE)
            return false;

        // If another shop source in the same node contains the same item, deny item placement
        WorldNode* shop_node = _world.node(source->node_id());
        const std::vector<ItemSource*> sources_in_node = shop_node->item_sources();
        for(ItemSource* source_in_node : sources_in_node)
            if(source_in_node != source && source_in_node->is_shop_item() && source_in_node->item() == item)
                return false;
    }

    return true;
}

/**
 * Build a list containing the paths currently being blocked as identified by the WorldSolver.
 * This list is weighted (contains as many instances of a WorldPath as its "weight" attribute) and shuffled.
 * @return a list of blocked paths
 */
std::vector<WorldPath*> WorldShuffler::build_weighted_blocked_paths_list()
{
    const std::vector<WorldPath*>& blocked_paths = _solver.blocked_paths();
    std::vector<WorldPath*> weighted_blocked_paths;

    for (WorldPath* path : blocked_paths)
    {
        // If items are not the (only) blocking point for taking this path, let it go
        if(!_solver.missing_nodes_to_take_path(path).empty())
            continue;

        // If all items cannot be placed since the item pool is running out of that item type,
        // do not try to open this path. It will open by itself once the item (as placed inside plando)
        // will be reached.
        std::vector<Item*> items_to_place = _solver.missing_items_to_take_path(path);
        std::map<uint8_t, uint16_t> quantities_to_place;
        for(Item* item : items_to_place)
        {
            if(!quantities_to_place.count(item->id()))
                quantities_to_place[item->id()] = 0;
            quantities_to_place[item->id()] += 1;
        }

        bool can_place_all_items = true;
        for(auto& [item_id, quantity_to_place] : quantities_to_place)
        {
            if(_item_pool_quantities[item_id] < quantity_to_place)
            {
                can_place_all_items = false;
                break;
            }
        }

        if(can_place_all_items)
        {
            for(int i=0 ; i<path->weight() ; ++i)
                weighted_blocked_paths.emplace_back(path);

            // All conditions are met, add this path to the weighted blocked paths list
            Json& debug_log = _solver.debug_log_for_current_step();
            debug_log["blockedPaths"].emplace_back(path->origin()->id() + " --> " + path->destination()->id());
        }
    }

    tools::shuffle(weighted_blocked_paths, _rng);
    return std::move(weighted_blocked_paths);
}

/**
 * Open one of the blocked paths encountered by the WorldSolver by placing randomly the items required to cross it.
 */
void WorldShuffler::open_random_blocked_path()
{
    Json& debug_log = _solver.debug_log_for_current_step();
    debug_log["blockedPaths"] = Json::array();

    // List all blocked paths, taking weights into account and randomly choose one
    std::vector<WorldPath*> weighted_blocked_paths = this->build_weighted_blocked_paths_list();
    if (weighted_blocked_paths.empty())
        return;
    WorldPath* path_to_open = weighted_blocked_paths[0];
    debug_log["chosenPath"].emplace_back(path_to_open->origin()->id() + " --> " + path_to_open->destination()->id());

    // Place all missing key items for the player to be able to open this blocking path
    std::vector<Item*> items_to_place = _solver.missing_items_to_take_path(path_to_open);
    for(Item* item : items_to_place)
    {
        while(_item_pool_quantities[item->id()] > 0)
        {
            ItemSource* source = this->place_item_randomly(item, _solver.empty_reachable_item_sources());
            _logical_playthrough.emplace_back(source);
            debug_log["placedKeyItems"][source->name()] = item->name();
        }
    }
}

/**
 * Place all remaining Items from the item pool in the ItemSources that are still empty.
 * Since the item pool is build in a way to enforce having the exact same number of Items and ItemSources, the
 * count matching should be perfect inside this function.
 */
void WorldShuffler::place_remaining_items()
{
    Json& debug_log = _solver.debug_log_for_current_step();

    std::vector<ItemSource*> unrestricted_item_sources, restricted_item_sources;
    for(ItemSource* source : _world.item_sources())
    {
        if(!source->empty())
            continue;

        if(source->is_chest() || source->is_npc_reward())
            unrestricted_item_sources.emplace_back(source);
        else
            restricted_item_sources.emplace_back(source);
    }

    // Step 1: We fill "restricted" (not compatible with all items) item sources with the first compatible item
    for(ItemSource* source : restricted_item_sources)
        this->fill_item_source_randomly(source);

    // Step 2: We fill unrestricted item sources (= chests & NPCs)
    for(ItemSource* source : unrestricted_item_sources)
        this->fill_item_source_randomly(source);

    for(Item* item : _item_pool)
        std::cout << "[WARNING] Item '" << item->name() << "' is remaining in the item pool at end of generation.\n";

    _item_pool.clear();
}


///////////////////////////////////////////////////////////////////////////////////////
///        THIRD PASS RANDOMIZATIONS (after items)
///////////////////////////////////////////////////////////////////////////////////////

void WorldShuffler::randomize_hints()
{
    this->init_hint_collections();

    this->randomize_lithograph_hint();
    this->randomize_where_is_lithograph_hint();

    Item* hinted_item =this->randomize_fortune_teller_hint();
    this->randomize_oracle_stone_hint(hinted_item);

    this->randomize_fox_hints();
}

void WorldShuffler::init_hint_collections()
{
    // A shuffled list of regions, used for the "barren / useful node" hints
    for(WorldRegion* region : _world.regions())
        if(region->can_be_hinted_as_required())
            _hintable_region_requirements.emplace_back(region);
    tools::shuffle(_hintable_region_requirements, _rng);

    // A shuffled list of potentially optional items, useful for the "this item will be useful / useless" hints
    _hintable_item_requirements = {
            ITEM_BUYER_CARD,   ITEM_EINSTEIN_WHISTLE,   ITEM_ARMLET,    ITEM_GARLIC,
            ITEM_IDOL_STONE,   ITEM_CASINO_TICKET,      ITEM_LOGS,      ITEM_LANTERN,
            ITEM_SUN_STONE
    };
    tools::shuffle(_hintable_item_requirements, _rng);

    // A shuffled list of items which location is interesting, useful for the "item X is in Y" hints
    _hintable_item_locations = {
            ITEM_SPIKE_BOOTS,       ITEM_AXE_MAGIC,      ITEM_BUYER_CARD,    ITEM_GARLIC,
            ITEM_EINSTEIN_WHISTLE,  ITEM_ARMLET,         ITEM_IDOL_STONE,    ITEM_LANTERN,
            ITEM_SUN_STONE,         ITEM_KEY,            ITEM_SAFETY_PASS,   ITEM_LOGS,
            ITEM_GOLA_EYE,          ITEM_GOLA_NAIL,      ITEM_GOLA_FANG,     ITEM_GOLA_HORN
    };
    tools::shuffle(_hintable_item_locations, _rng);
}

void WorldShuffler::randomize_lithograph_hint()
{
    HintSource* lithograph_hint_source = _world.hint_source("Lithograph");

    // If hint source already contains text (e.g. through plando descriptor), ignore it
    if(!lithograph_hint_source->text().empty())
    {
        _world.add_used_hint_source(lithograph_hint_source);
        return;
    }

    if(_options.jewel_count() == 0 || _world.item_distribution(ITEM_LITHOGRAPH)->quantity() == 0)
    {
        lithograph_hint_source->text("This tablet seems of no use...");
        return;
    }

    _world.add_used_hint_source(lithograph_hint_source);

    std::stringstream lithograph_hint;
    if(_options.jewel_count() > MAX_INDIVIDUAL_JEWELS)
    {
        bool first = true;
        std::vector<ItemSource*> sources_containing_jewels = _world.item_sources_with_item(_world.item(ITEM_RED_JEWEL));
        for(ItemSource* source : sources_containing_jewels)
        {
            if(first)
                first = false;
            else
                lithograph_hint << "\n";
            lithograph_hint << "A jewel is " << this->random_hint_for_item_source(source) << ".";
        }
    }
    else
    {
        lithograph_hint << "Red Jewel is " << this->random_hint_for_item(_world.item(ITEM_RED_JEWEL)) << ".";
        if(_options.jewel_count() >= 2)
            lithograph_hint << "\nPurple Jewel is " << this->random_hint_for_item(_world.item(ITEM_PURPLE_JEWEL)) << ".";
        if(_options.jewel_count() >= 3)
            lithograph_hint << "\nGreen Jewel is " << this->random_hint_for_item(_world.item(ITEM_GREEN_JEWEL)) << ".";
        if(_options.jewel_count() >= 4)
            lithograph_hint << "\nBlue Jewel is " << this->random_hint_for_item(_world.item(ITEM_BLUE_JEWEL)) << ".";
        if(_options.jewel_count() >= 5)
            lithograph_hint << "\nYellow Jewel is " << this->random_hint_for_item(_world.item(ITEM_YELLOW_JEWEL)) << ".";
    }

    lithograph_hint_source->text(lithograph_hint.str());
}

void WorldShuffler::randomize_where_is_lithograph_hint()
{
    HintSource* knc_sign_source = _world.hint_source("King Nole's Cave sign");

    _world.add_used_hint_source(knc_sign_source);

    if(!knc_sign_source->text().empty())
        return;

    std::stringstream where_is_litho_hint;
    where_is_litho_hint << "The lithograph will help you finding the jewels. It is "
                        << this->random_hint_for_item(_world.item(ITEM_LITHOGRAPH))
                        << ".";

    knc_sign_source->text(where_is_litho_hint.str());
}

Item* WorldShuffler::randomize_fortune_teller_hint()
{
    HintSource* fortune_teller_source = _world.hint_source("Mercator fortune teller");

    _world.add_used_hint_source(fortune_teller_source);

    // If hint source already contains text (e.g. through plando descriptor), ignore it
    if(!fortune_teller_source->text().empty())
        return nullptr;

    std::vector<uint8_t> hintable_items = { ITEM_GOLA_EYE, ITEM_GOLA_NAIL, ITEM_GOLA_FANG, ITEM_GOLA_HORN };
    tools::shuffle(hintable_items, _rng);
    
    Item* hinted_item = _world.item(*(hintable_items.begin()));

    std::string item_fancy_name;
    if (hinted_item == _world.item(ITEM_GOLA_EYE))
        item_fancy_name = "an eye";
    else if (hinted_item == _world.item(ITEM_GOLA_NAIL))
        item_fancy_name = "a nail";
    else if (hinted_item == _world.item(ITEM_GOLA_FANG))
        item_fancy_name = "a fang";
    else if (hinted_item == _world.item(ITEM_GOLA_HORN))
        item_fancy_name = "a horn";

    std::stringstream fortune_teller_hint;
    fortune_teller_hint << "\x1cI see... \x1aI see... \x1a\nI see " << item_fancy_name << " " << this->random_hint_for_item(hinted_item) << ".";
    fortune_teller_source->text(fortune_teller_hint.str());

    _hintable_item_locations.erase(hinted_item->id());
    return hinted_item;
}

void WorldShuffler::randomize_oracle_stone_hint(Item* forbidden_fortune_teller_item)
{
    HintSource* oracle_stone_source = _world.hint_source("Oracle Stone");

    if(_world.item_distribution(ITEM_ORACLE_STONE)->quantity() > 0)
    {
        _world.add_used_hint_source(oracle_stone_source);

        // If hint source already contains text (e.g. through plando descriptor), ignore it
        if(!oracle_stone_source->text().empty())
            return;

        UnsortedSet<Item*> forbidden_items = {
                forbidden_fortune_teller_item, _world.item(ITEM_RED_JEWEL), _world.item(ITEM_PURPLE_JEWEL),
                _world.item(ITEM_GREEN_JEWEL), _world.item(ITEM_BLUE_JEWEL), _world.item(ITEM_YELLOW_JEWEL)
        };

        // Also excluding items strictly needed to get to Oracle Stone's location
        std::vector<ItemSource*> sources = _world.item_sources_with_item(_world.item(ITEM_ORACLE_STONE));
        WorldNode* first_source_node = _world.node(sources.at(0)->node_id());
        if(first_source_node)
        {
            WorldSolver solver(_world);
            if(solver.try_to_solve(_world.spawn_node(), first_source_node, _world.starting_inventory()))
            {
                std::vector<Item*> min_items_to_reach = solver.find_minimal_inventory();
                for(Item* item : min_items_to_reach)
                    forbidden_items.insert(item);
            }
            else
            {
                tools::dump_json_to_file(_solver.debug_log(), "./debug.json");
                throw LandstalkerException("Could not find minimal inventory to reach Oracle Stone");
            }
        }

        std::vector<Item*> hintable_items;
        for(Item* item : _minimal_items_to_complete)
        {
            if(!forbidden_items.contains(item))
                hintable_items.emplace_back(item);
        }

        if(!hintable_items.empty())
        {
            tools::shuffle(hintable_items, _rng);
            Item* hinted_item = *hintable_items.begin();

            std::stringstream oracle_stone_hint;
            oracle_stone_hint << "You will need " << hinted_item->name() << ". It is "
                              << this->random_hint_for_item(hinted_item) << ".";
            oracle_stone_source->text(oracle_stone_hint.str());

            _hintable_item_requirements.erase(hinted_item->id());
            _hintable_item_locations.erase(hinted_item->id());
            return;
        }
    }

    oracle_stone_source->text("The stone looks blurry. It looks like it won't be of any use...");
}

void WorldShuffler::randomize_fox_hints()
{
    uint8_t hints_count = _options.hints_count();
    for(HintSource* source : _world.used_hint_sources())
        if(source->has_entity())
            --hints_count;

    // Pick a subset of all possible fox_hints trying to follow as much as possible the randomizer settings
    std::vector<HintSource*> foxes_pool;
    for(HintSource* hint_source : _world.hint_sources())
    {
        // If source is already used (e.g. in plando context), don't add it to the pool
        if(std::find(_world.used_hint_sources().begin(), _world.used_hint_sources().end(), hint_source) != _world.used_hint_sources().end())
            continue;

        if(hint_source->has_entity())
            foxes_pool.emplace_back(hint_source);
    }
    tools::shuffle(foxes_pool, _rng);
    if(foxes_pool.size() > hints_count)
        foxes_pool.resize(hints_count);

    bool has_hinted_dark_region = false;

    // Put hints inside
    for (HintSource* hint_source : foxes_pool)
    {
        _world.add_used_hint_source(hint_source);

        // If hint source already contains text (e.g. through plando descriptor), ignore it
        if(!hint_source->text().empty())
            continue;

        // Fallback in case none of the following options match.
        hint_source->text("I don't have anything to tell you. Move on.");

        if(_options.hint_dark_region() && !has_hinted_dark_region)
        {
            if(generate_dark_region_hint(hint_source))
            {
                has_hinted_dark_region = true;
                continue;
            }
        }

        double random_number = (double) _rng() / (double) std::mt19937::max();
        double current_tested_value = _options.hint_distribution_region_requirement();
        if(random_number < current_tested_value)
        {
            // "Barren / pleasant surprise"
            if(!_hintable_region_requirements.empty())
                this->generate_region_requirement_hint(hint_source);
            continue;
        }

        current_tested_value += _options.hint_distribution_item_requirement();
        if(random_number < current_tested_value)
        {
            // "You will / won't need {item} to finish"
            if(!_hintable_item_requirements.empty())
                this->generate_item_requirement_hint(hint_source);
            continue;
        }

        current_tested_value += _options.hint_distribution_item_location();
        if(random_number < current_tested_value)
        {
            // "You shall find {item} in {place}"
            if(!_hintable_item_locations.empty())
                this->generate_item_position_hint(hint_source);
            continue;
        }
    }
}

bool WorldShuffler::generate_dark_region_hint(HintSource* hint_source)
{
    WorldRegion* region = _world.dark_region();
    WorldSolver solver(_world);
    solver.forbid_taking_items_from_nodes(region->nodes());
    if(!solver.try_to_solve(_world.spawn_node(), hint_source->node(), _world.starting_inventory()))
        return false;

    hint_source->text("If you find yourself " + region->hint_name() + " without a lantern, all you will find is darkness.");
    return true;
}

void WorldShuffler::generate_region_requirement_hint(HintSource* hint_source)
{
    WorldRegion* region = *_hintable_region_requirements.begin();

    if (this->is_region_avoidable(region))
        hint_source->text("What you are looking for is not " + region->hint_name() + ".");
    else
        hint_source->text("You might have a pleasant surprise wandering " + region->hint_name() + ".");

    _hintable_region_requirements.erase(region);
}

bool WorldShuffler::generate_item_requirement_hint(HintSource* hint_source)
{
    Item* hinted_item_requirement = nullptr;
    for(uint8_t item_id : _hintable_item_requirements)
    {
        Item* tested_item = _world.item(item_id);

        WorldSolver solver(_world);
        solver.forbid_item_type(tested_item);
        if(solver.try_to_solve(_world.spawn_node(), hint_source->node(), _world.starting_inventory()))
        {
            // If item is not mandatory to reach the hint source, we can hint it
            hinted_item_requirement = tested_item;
            _hintable_item_requirements.erase(item_id);
            break;
        }
    }

    if(!hinted_item_requirement)
        return false;

    if (!this->is_item_avoidable(hinted_item_requirement))
        hint_source->text("You will need " + hinted_item_requirement->name() + " in your quest to King Nole's treasure.");
    else
        hint_source->text(hinted_item_requirement->name() + " is not required in your quest to King Nole's treasure.");

    return true;
}

bool WorldShuffler::generate_item_position_hint(HintSource* hint_source)
{
    Item* hinted_item_location = nullptr;
    for(uint8_t item_id : _hintable_item_locations)
    {
        Item* tested_item = _world.item(item_id);

        WorldSolver solver(_world);
        solver.forbid_item_type(tested_item);
        if(solver.try_to_solve(_world.spawn_node(), hint_source->node(), _world.starting_inventory()))
        {
            // If item is not mandatory to reach the hint source, we can hint it
            hinted_item_location = tested_item;
            _hintable_item_locations.erase(item_id);
            break;
        }
    }

    if (!hinted_item_location)
        return false;

    hint_source->text("You shall find " + hinted_item_location->name() + " " + this->random_hint_for_item(hinted_item_location) + ".");
    return true;
}

std::string WorldShuffler::random_hint_for_item(Item* item)
{
    std::vector<ItemSource*> sources = _world.item_sources_with_item(item);
    if(sources.empty())
        return "in an unknown place";

    tools::shuffle(sources, _rng);
    ItemSource* randomSource = sources[0];
    return this->random_hint_for_item_source(randomSource);
}

std::string WorldShuffler::random_hint_for_item_source(ItemSource* itemSource)
{
    const std::vector<std::string>& node_hints = _world.node(itemSource->node_id())->hints();
    const std::vector<std::string>& source_hints = itemSource->hints();
    
    std::vector<std::string> all_hints;
    all_hints.insert(all_hints.end(), node_hints.begin(), node_hints.end());
    all_hints.insert(all_hints.end(), source_hints.begin(), source_hints.end());
    
    if(all_hints.empty())
        return "in an unknown place";

    tools::shuffle(all_hints, _rng);
    return all_hints[0];
}

bool WorldShuffler::is_region_avoidable(WorldRegion* region) const
{
    WorldSolver solver(_world);
    solver.forbid_taking_items_from_nodes(region->nodes());
    return solver.try_to_solve(_world.spawn_node(), _world.end_node(), _world.starting_inventory());
}

bool WorldShuffler::is_item_avoidable(Item* item) const
{
    WorldSolver solver(_world);
    solver.forbid_item_type(item);
    return solver.try_to_solve(_world.spawn_node(), _world.end_node(), _world.starting_inventory());
}

Json WorldShuffler::playthrough_as_json() const
{
    Json json;

    // Filter the logical playthrough to keep only strictly needed key items
    for(ItemSource* source : _logical_playthrough)
    {
        Item* key_item_in_source = source->item();
        if(std::find(_minimal_items_to_complete.begin(), _minimal_items_to_complete.end(), key_item_in_source) != _minimal_items_to_complete.end())
            json[source->name()] = key_item_in_source->name();
    }

    return json;
}


