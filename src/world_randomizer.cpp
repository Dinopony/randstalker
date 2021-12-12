#include "world_randomizer.hpp"

#include <landstalker_lib/constants/entity_type_codes.hpp>
#include <landstalker_lib/constants/item_codes.hpp>
#include <landstalker_lib/constants/values.hpp>
#include <landstalker_lib/tools/tools.hpp>
#include <landstalker_lib/tools/game_text.hpp>
#include <landstalker_lib/model/entity_type.hpp>
#include <landstalker_lib/model/item_source.hpp>
#include <landstalker_lib/model/world_teleport_tree.hpp>
#include <landstalker_lib/model/spawn_location.hpp>
#include <landstalker_lib/model/world.hpp>
#include <landstalker_lib/exceptions.hpp>

#include "logic_model/hint_source.hpp"
#include "logic_model/world_region.hpp"
#include "logic_model/item_distribution.hpp"
#include "world_solver.hpp"

#include <algorithm>
#include <iostream>

WorldRandomizer::WorldRandomizer(World& world, WorldLogic& logic, const RandomizerOptions& options) :
    _world          (world),
    _logic          (logic),
    _solver         (_logic),
    _options        (options),
    _rng            (_options.seed())
{}

void WorldRandomizer::randomize()
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
}

///////////////////////////////////////////////////////////////////////////////////////
///        FIRST PASS RANDOMIZATIONS (before items)
///////////////////////////////////////////////////////////////////////////////////////

void WorldRandomizer::randomize_spawn_location()
{
    if(!_world.spawn_location().empty())
        return;

    std::vector<std::string> possible_spawn_locations = _options.possible_spawn_locations();
    if(possible_spawn_locations.empty())
    {
        for(auto& [id, spawn] : _logic.spawn_locations())
            possible_spawn_locations.emplace_back(id);
    }

    tools::shuffle(possible_spawn_locations, _rng);
    SpawnLocation* spawn = _logic.spawn_locations().at(possible_spawn_locations[0]);
    _logic.active_spawn_location(spawn, _world);
}

void WorldRandomizer::randomize_dark_rooms()
{
    // If dark region has already been set (e.g. through plando descriptor), no need to set it here
    if(_logic.dark_region())
        return;

    std::vector<Item*> starting_inventory = _world.starting_inventory();
    Item* item_lantern = _world.item(ITEM_LANTERN);
    bool lantern_as_starting_item = std::find(starting_inventory.begin(), starting_inventory.end(), item_lantern) != starting_inventory.end();

    std::vector<WorldRegion*> possible_regions;
    for (WorldRegion* region : _logic.regions())
    {
        // Don't allow spawning inside a dark node, unless we have lantern as starting item
        if(!lantern_as_starting_item && region == _logic.spawn_node()->region())
            continue;

        if (!region->dark_map_ids().empty())
            possible_regions.emplace_back(region);
    }

    tools::shuffle(possible_regions, _rng);
    WorldRegion* dark_region = possible_regions[0];
    _logic.dark_region(dark_region, _world);
}

void WorldRandomizer::randomize_tibor_trees()
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

void WorldRandomizer::randomize_fahl_enemies()
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

void WorldRandomizer::init_item_pool()
{
    _item_pool_quantities = _logic.item_quantities();

    // Count quantities already in place
    for(ItemSource* source : _world.item_sources())
    {
        Item* item = source->item();
        if(item)
        {
            if(_item_pool_quantities[item->id()] == 0)
            {
                throw LandstalkerException("There are more " + item->name() +
                                           " already placed than the expected number in the item pool");
            }
            _item_pool_quantities[item->id()] -= 1;
        }
    }

    _item_pool.clear();
    _item_pool.reserve(_world.item_sources().size());

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

    size_t empty_item_sources_count = 0;
    for(ItemSource* source : _world.item_sources())
        if(source->item() == nullptr)
            empty_item_sources_count += 1;

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

    tools::shuffle(_item_pool, _rng);
}

void WorldRandomizer::remove_item_from_pool(Item* item)
{
    _item_pool.erase(std::find(_item_pool.begin(), _item_pool.end(), item));
    _item_pool_quantities[item->id()] -= 1;
}

Item* WorldRandomizer::generate_gold_item()
{
    std::normal_distribution<double> distribution(40.0, 18.0);
    double gold_value = distribution(_rng);

    if (gold_value < 1)
        gold_value = 1;
    else if (gold_value > 255)
        gold_value = 255;

    return _world.add_gold_item(static_cast<uint8_t>(gold_value));
}

void WorldRandomizer::randomize_items()
{
    this->init_item_pool();

    _solver.setup(_logic.spawn_node(), _logic.end_node(), _world.starting_inventory());

    bool explored_new_nodes = true;
    while(explored_new_nodes)
    {
        // Run a solver step to reach a "blocked" state where something needs to be placed in order to continue
        explored_new_nodes = _solver.run_until_blocked();
        
        std::vector<ItemSource*> empty_sources = _solver.empty_reachable_item_sources();
        tools::shuffle(empty_sources, _rng);

        this->place_key_items(empty_sources);

        // Item sources changed, force the solver to update its inventory
        _solver.update_current_inventory();
    }

    // Place the remaining items from the item pool in the remaining sources
    std::vector<ItemSource*> empty_sources;
    for(ItemSource* source : _world.item_sources())
        if(!source->item())
            empty_sources.emplace_back(source);
    this->place_remaining_items(empty_sources);

    // Analyse items required to complete the seed
    Json& debug_log = _solver.debug_log();
    _minimal_items_to_complete = _solver.find_minimal_inventory();
    debug_log["requiredItems"] = Json::array();
    for (Item* item : _minimal_items_to_complete)
        debug_log["requiredItems"].emplace_back(item->name());
}

bool WorldRandomizer::test_item_source_compatibility(ItemSource* source, Item* item) const
{
    if(source->is_chest())
        return true;

    if(source->is_npc_reward())
        return (item->id() != ITEM_NONE);

    ItemSourceOnGround* cast_source = reinterpret_cast<ItemSourceOnGround*>(source);
    if(item->id() >= ITEM_GOLDS_START)
        return false;
    if(!cast_source->can_be_taken_only_once() && !_logic.item_distribution(item->id())->allowed_on_ground())
        return false;

    if(source->is_shop_item())
    {
        if(item->id() == ITEM_NONE)
            return false;

        // If another shop source in the same node contains the same item, deny item placement
        WorldNode* shop_node = _logic.node(source->node_id());
        const std::vector<ItemSource*> sources_in_node = shop_node->item_sources();
        for(ItemSource* source_in_node : sources_in_node)
            if(source_in_node != source && source_in_node->is_shop_item() && source_in_node->item() == item)
                return false;
    }

    return true;
}

ItemSource* WorldRandomizer::pop_first_compatible_source(std::vector<ItemSource*>& sources, Item* item)
{
    for (uint32_t i = 0; i < sources.size(); ++i)
    {
        if (this->test_item_source_compatibility(sources[i], item))
        {
            ItemSource* source = sources[i];
            sources.erase(sources.begin() + i);
            return source;
        }
    }

    return nullptr;
}

void WorldRandomizer::place_key_items(std::vector<ItemSource*>& empty_sources)
{
    Json& debug_log = _solver.debug_log_for_current_step();
    debug_log["blockedPaths"] = Json::array();

    // List all blocked paths, taking weights into account
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

        if(!can_place_all_items)
            continue;

        // All conditions are met, add this path to the weighted blocked paths list
        debug_log["blockedPaths"].emplace_back(path->origin()->id() + " --> " + path->destination()->id());
        for(int i=0 ; i<path->weight() ; ++i)
            weighted_blocked_paths.emplace_back(path);
    }

    if (weighted_blocked_paths.empty())
        return;

    // Randomly choose one of those blocking paths
    tools::shuffle(weighted_blocked_paths, _rng);
    WorldPath* path_to_open = weighted_blocked_paths[0];
    debug_log["chosenPath"].emplace_back(path_to_open->origin()->id() + " --> " + path_to_open->destination()->id());

    // Place all missing key items for the player to be able to open this blocking path
    std::vector<Item*> items_to_place = _solver.missing_items_to_take_path(path_to_open);
    std::vector<Item*> extra_items = path_to_open->items_placed_when_crossing();
    items_to_place.insert(items_to_place.end(), extra_items.begin(), extra_items.end());
    for(Item* item : items_to_place)
    {
        while(_item_pool_quantities[item->id()] > 0)
        {
            this->remove_item_from_pool(item);

            // Place the key item in a compatible source
            ItemSource* compatible_source = this->pop_first_compatible_source(empty_sources, item);
            if(!compatible_source)
                throw LandstalkerException("No appropriate item source found for placing key item");

            compatible_source->item(item);
            _logical_playthrough.emplace_back(compatible_source);
            debug_log["placedKeyItems"][compatible_source->name()] = item->name();
        }
    }
}

void WorldRandomizer::place_remaining_items(std::vector<ItemSource*>& empty_sources)
{
    Json& debug_log = _solver.debug_log_for_current_step();

    std::vector<ItemSource*> empty_chests;
    std::vector<ItemSource*> constrained_item_sources;
    for(ItemSource* source : empty_sources)
    {
        if(source->is_chest())
            empty_chests.emplace_back(source);
        else
            constrained_item_sources.emplace_back(source);
    }

    // Step 1: We fill "constrained" item sources with the first compatible item
    for(ItemSource* source : constrained_item_sources)
    {
        for(auto it=_item_pool.begin() ; it!=_item_pool.end() ; ++it)
        {
            if(test_item_source_compatibility(source, *it))
            {
                source->item(*it);
                _item_pool.erase(it);
                break;
            }
        }
    }

    for(ItemSource* source : constrained_item_sources)
        if(source->item() == nullptr)
            std::cout << "[WARNING] Item source '" << source->name() << "' could not be filled with any item of the item pool.\n";

    // Step 2: We fill chests (= unrestricted)
    for(ItemSource* chest : empty_chests)
    {
        chest->item(_item_pool[0]);
        _item_pool.erase(_item_pool.begin());
    }

    for(Item* item : _item_pool)
        std::cout << "[WARNING] Item '" << item->name() << "' is remaining in the item pool at end of generation.\n";

    _item_pool.clear();
}


///////////////////////////////////////////////////////////////////////////////////////
///        THIRD PASS RANDOMIZATIONS (after items)
///////////////////////////////////////////////////////////////////////////////////////

void WorldRandomizer::randomize_hints()
{
    this->randomize_lithograph_hint();
    this->randomize_where_is_lithograph_hint();

    Item* hinted_fortune_item = this->randomize_fortune_teller_hint();
    Item* hinted_oracle_stone_item = this->randomize_oracle_stone_hint(hinted_fortune_item);

    this->randomize_sign_hints(hinted_fortune_item, hinted_oracle_stone_item);
}

void WorldRandomizer::randomize_lithograph_hint()
{
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
    else if(_options.jewel_count() >= 1)
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
    else
        lithograph_hint << "This tablet seems of no use...";

    _logic.hint_source("Lithograph")->text(lithograph_hint.str());
}

void WorldRandomizer::randomize_where_is_lithograph_hint()
{
    HintSource* knc_sign_source = _logic.hint_source("King Nole's Cave sign");
    if(!knc_sign_source->text().empty())
        return;

    std::stringstream where_is_litho_hint;
    where_is_litho_hint << "The lithograph will help you finding the jewels. It is "
                        << this->random_hint_for_item(_world.item(ITEM_LITHOGRAPH))
                        << ".";

    knc_sign_source->text(where_is_litho_hint.str());
}

Item* WorldRandomizer::randomize_fortune_teller_hint()
{
    HintSource* fortune_teller_source = _logic.hint_source("Mercator fortune teller");
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

    return hinted_item;
}

Item* WorldRandomizer::randomize_oracle_stone_hint(Item* forbidden_fortune_teller_item)
{
    HintSource* oracle_stone_source = _logic.hint_source("Oracle Stone");
    if(!oracle_stone_source->text().empty())
        return nullptr;

    if(_logic.item_distribution(ITEM_ORACLE_STONE)->quantity() > 0)
    {
        UnsortedSet<Item*> forbidden_items = {
                forbidden_fortune_teller_item, _world.item(ITEM_RED_JEWEL), _world.item(ITEM_PURPLE_JEWEL),
                _world.item(ITEM_GREEN_JEWEL), _world.item(ITEM_BLUE_JEWEL), _world.item(ITEM_YELLOW_JEWEL)
        };

        // Also excluding items strictly needed to get to Oracle Stone's location
        std::vector<ItemSource*> sources = _world.item_sources_with_item(_world.item(ITEM_ORACLE_STONE));
        WorldNode* first_source_node = _logic.node(sources.at(0)->node_id());
        if(first_source_node)
        {
            WorldSolver solver(_logic);
            if(solver.try_to_solve(_logic.spawn_node(), first_source_node, _world.starting_inventory()))
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

            return hinted_item;
        }
    }

    oracle_stone_source->text("The stone looks blurry. It looks like it won't be of any use...");
    return nullptr;
}

void WorldRandomizer::randomize_sign_hints(Item* hinted_fortune_item, Item* hinted_oracle_stone_item)
{
    // A shuffled list of regions, used for the "barren / useful node" hints
    UnsortedSet<WorldRegion*> hintable_regions;
    for(WorldRegion* region : _logic.regions())
        if(region->can_be_hinted())
            hintable_regions.emplace_back(region);
    tools::shuffle(hintable_regions, _rng);

    // A shuffled list of potentially optional items, useful for the "this item will be useful / useless" hints
    UnsortedSet<uint8_t> hintable_item_requirements = {
        ITEM_BUYER_CARD,   ITEM_EINSTEIN_WHISTLE,   ITEM_ARMLET,    ITEM_GARLIC,
        ITEM_IDOL_STONE,   ITEM_CASINO_TICKET,      ITEM_LOGS,      ITEM_LANTERN,
        ITEM_SUN_STONE
    };
    tools::shuffle(hintable_item_requirements, _rng);

    // A shuffled list of items which location is interesting, useful for the "item X is in Y" hints
    UnsortedSet<uint8_t> hintable_item_locations = {
        ITEM_SPIKE_BOOTS,       ITEM_AXE_MAGIC,      ITEM_BUYER_CARD,    ITEM_GARLIC,
        ITEM_EINSTEIN_WHISTLE,  ITEM_ARMLET,         ITEM_IDOL_STONE,    ITEM_LANTERN,
        ITEM_SUN_STONE,         ITEM_KEY,            ITEM_SAFETY_PASS,   ITEM_LOGS,
        ITEM_GOLA_EYE,          ITEM_GOLA_NAIL,      ITEM_GOLA_FANG,     ITEM_GOLA_HORN
    };
    tools::shuffle(hintable_item_locations, _rng);

    // Remove items that have been already hinted by special hints
    if(hinted_fortune_item)
        hintable_item_locations.erase(hinted_fortune_item->id());
    if(hinted_oracle_stone_item)
    {
        hintable_item_requirements.erase(hinted_oracle_stone_item->id());
        hintable_item_locations.erase(hinted_oracle_stone_item->id());
    }

    for (auto& [k, hint_source] : _logic.hint_sources())
    {
        // If hint source is special (e.g. Oracle Stone, Lithograph...), don't handle it here
        // If it already contains text (e.g. through plando descriptor), ignore it
        if(hint_source->special() || !hint_source->text().empty())
            continue;
        
        double random_number = (double) _rng() / (double) std::mt19937::max();

        ////////////////////////////////////////////////////////////////
        // "Barren / pleasant surprise" (30%)
        if (random_number < 0.3 && !hintable_regions.empty())
        {
            WorldRegion* region = *hintable_regions.begin();

            if (this->is_region_avoidable(region))
                hint_source->text("What you are looking for is not " + region->hint_name() + ".");
            else
                hint_source->text("You might have a pleasant surprise wandering " + region->hint_name() + ".");

            hintable_regions.erase(region);
            continue;
        }

        ////////////////////////////////////////////////////////////////
        // "You will / won't need {item} to finish" (25%)
        if (random_number < 0.55)
        {
            Item* hinted_item_requirement = nullptr;
            for(uint8_t item_id : hintable_item_requirements)
            {
                Item* tested_item = _world.item(item_id);
                
                WorldSolver solver(_logic);
                solver.forbid_item_types({ tested_item });
                if(solver.try_to_solve(_logic.spawn_node(), hint_source->node(), _world.starting_inventory()))
                {
                    // If item is not mandatory to reach the hint source, we can hint it
                    hinted_item_requirement = tested_item;
                    hintable_item_requirements.erase(item_id);
                    break;
                }
            }

            if(hinted_item_requirement)
            {
                if (!this->is_item_avoidable(hinted_item_requirement))
                    hint_source->text("You will need " + hinted_item_requirement->name() + " in your quest to King Nole's treasure.");
                else
                    hint_source->text(hinted_item_requirement->name() + " is not required in your quest to King Nole's treasure.");
                continue;
            }
        }

        ////////////////////////////////////////////////////////////////
        // "You shall find {item} in {place}" (45%)
        if (!hintable_item_locations.empty())
        {
            Item* hinted_item_location = nullptr;
            for(uint8_t item_id : hintable_item_locations)
            {
                Item* tested_item = _world.item(item_id);

                WorldSolver solver(_logic);
                solver.forbid_item_types({ tested_item });
                if(solver.try_to_solve(_logic.spawn_node(), hint_source->node(), _world.starting_inventory()))
                {
                    // If item is not mandatory to reach the hint source, we can hint it
                    hinted_item_location = tested_item;
                    hintable_item_locations.erase(item_id);
                    break;
                }
            }

            if (hinted_item_location)
            {
                hint_source->text("You shall find " + hinted_item_location->name() + " " + this->random_hint_for_item(hinted_item_location) + ".");
                continue;
            }
        }

        // Fallback if none matched
        hint_source->text("This sign has been damaged in a way that makes it unreadable.");
    }
}

std::string WorldRandomizer::random_hint_for_item(Item* item)
{
    std::vector<ItemSource*> sources = _world.item_sources_with_item(item);
    if(sources.empty())
        return "in an unknown place";

    tools::shuffle(sources, _rng);
    ItemSource* randomSource = sources[0];
    return this->random_hint_for_item_source(randomSource);
}

std::string WorldRandomizer::random_hint_for_item_source(ItemSource* itemSource)
{
    const std::vector<std::string>& node_hints = _logic.node(itemSource->node_id())->hints();
    const std::vector<std::string>& source_hints = itemSource->hints();
    
    std::vector<std::string> all_hints;
    all_hints.insert(all_hints.end(), node_hints.begin(), node_hints.end());
    all_hints.insert(all_hints.end(), source_hints.begin(), source_hints.end());
    
    if(all_hints.empty())
        return "in an unknown place";

    tools::shuffle(all_hints, _rng);
    return all_hints[0];
}

bool WorldRandomizer::is_region_avoidable(WorldRegion* region) const
{
    WorldSolver solver(_logic);
    solver.forbid_taking_items_from_nodes(region->nodes());
    return solver.try_to_solve(_logic.spawn_node(), _logic.end_node(), _world.starting_inventory());
}

bool WorldRandomizer::is_item_avoidable(Item* item) const
{
    WorldSolver solver(_logic);
    solver.forbid_item_types({ item });
    return solver.try_to_solve(_logic.spawn_node(), _logic.end_node(), _world.starting_inventory());
}

Json WorldRandomizer::playthrough_as_json() const
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


