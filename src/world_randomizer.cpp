#include "world_randomizer.hpp"

#include <algorithm>
#include <sstream>

#include "tools/tools.hpp"
#include "tools/game_text.hpp"
#include "model/enemy.hpp"

#include "exceptions.hpp"
#include "world_solver.hpp"

WorldRandomizer::WorldRandomizer(World& world, const RandomizerOptions& options) :
    _world            (world),
    _options          (options),
    _rng              (_options.seed()),
    _solver           ()
{}

void WorldRandomizer::randomize()
{
    // 1st pass: randomizations happening BEFORE randomizing items
    this->randomize_spawn_location();
    this->randomize_dark_rooms();
    
    if(_options.shuffle_tibor_trees())
        this->randomize_tibor_trees();
    if(_options.all_trees_visited_at_start())
        _world.add_tree_logic_paths();

    this->randomize_fahl_enemies();

    // 2nd pass: randomizing items
    this->init_mandatory_items();
    this->init_filler_items();
    this->randomize_items();

    // 3rd pass: randomizations happening AFTER randomizing items
    this->randomize_hints();
}

///////////////////////////////////////////////////////////////////////////////////////
///        FIRST PASS RANDOMIZATIONS (before items)
///////////////////////////////////////////////////////////////////////////////////////

void WorldRandomizer::randomize_spawn_location()
{
    std::vector<std::string> possible_spawn_locations = _options.possible_spawn_locations();
    if(possible_spawn_locations.empty())
    {
        for(auto& [id, spawn] : _world.spawn_locations())
            possible_spawn_locations.push_back(id);
    }

    tools::shuffle(possible_spawn_locations, _rng);
    SpawnLocation* spawn = _world.spawn_locations().at(possible_spawn_locations[0]);
    _world.active_spawn_location(spawn);
}

void WorldRandomizer::randomize_dark_rooms()
{
    std::vector<Item*> starting_inventory = _world.starting_inventory();
    Item* item_lantern = _world.item(ITEM_LANTERN);
    bool lantern_as_starting_item = std::find(starting_inventory.begin(), starting_inventory.end(), item_lantern) != starting_inventory.end();

    std::vector<WorldRegion*> possible_regions;
    for (auto& [key, region] : _world.regions())
    {
        // Don't allow spawning inside a dark region, unless we have lantern as starting item
        if(!lantern_as_starting_item && region == _world.spawn_region())
            continue;

        if (!region->dark_map_ids().empty())
            possible_regions.push_back(region);
    }

    tools::shuffle(possible_regions, _rng);
    _world.dark_region(possible_regions[0]);

    const std::vector<WorldPath*>& ingoing_paths = _world.dark_region()->ingoing_paths();
    for (WorldPath* path : ingoing_paths)
        path->add_required_item(_world.item(ITEM_LANTERN));
}

void WorldRandomizer::randomize_tibor_trees()
{
    const std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>>& tree_pairs = _world.teleport_tree_pairs();
    
    std::vector<std::pair<uint16_t, uint16_t>> map_id_pairs;
    std::vector<WorldTeleportTree*> trees;
    for(auto& pair : tree_pairs)
    {
        map_id_pairs.push_back(std::make_pair(pair.first->tree_map_id(), pair.second->tree_map_id()));
        trees.push_back(pair.first);
        trees.push_back(pair.second);
    }

    tools::shuffle(trees, _rng);

    std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>> new_pairs;
    for(size_t i=0 ; i<trees.size() ; i+=2)
    {
        new_pairs.push_back(std::make_pair(trees[i], trees[i+1]));
        const std::pair<uint16_t, uint16_t>& map_id_pair = map_id_pairs[i/2];
        trees[i]->tree_map_id(map_id_pair.first);
        trees[i+1]->tree_map_id(map_id_pair.second);
    }

    _world.teleport_tree_pairs(new_pairs);
}

void WorldRandomizer::randomize_fahl_enemies()
{
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

    _world.add_fahl_enemy(_world.enemy(easy_enemies[0]));
    _world.add_fahl_enemy(_world.enemy(easy_enemies[1]));
    _world.add_fahl_enemy(_world.enemy(medium_enemies[0]));
    _world.add_fahl_enemy(_world.enemy(medium_enemies[1]));
    _world.add_fahl_enemy(_world.enemy(hard_enemies[0]));
}


///////////////////////////////////////////////////////////////////////////////////////
///        SECOND PASS RANDOMIZATIONS (items)
///////////////////////////////////////////////////////////////////////////////////////

void WorldRandomizer::init_filler_items()
{
    std::map<std::string, uint16_t> filler_items_desc;
    if(_options.has_custom_filler_items())
    {
        filler_items_desc = _options.filler_items();
    }
    else
    {
        filler_items_desc = { 
            {"Life Stock", 80},      {"EkeEke", 55},         {"Golds", 30},          {"Dahl", 16},             
            {"Statue of Gaia", 12},  {"Detox Grass", 11},    {"Golden Statue", 10},  {"Restoration", 10},     
            {"Mind Repair", 7},      {"Anti Paralyze", 7},   {"No Item", 4},         {"Pawn Ticket", 1},
            {"Short Cake", 1},       {"Bell", 1},            {"Blue Ribbon", 1},     {"Death Statue", 1}
        };
    }

    uint8_t gold_items_count = 0;
    for (auto& [item_name, quantity] : filler_items_desc)
    {
        if(item_name == "Golds")
        {
            gold_items_count += quantity;
            continue;
        }

        Item* item = _world.item(item_name);
        if(!item)
        {
            std::stringstream msg;
            msg << "Unknown item '" << item_name << "' found in filler items.";
            throw RandomizerException(msg.str());
        }
        
        for(uint16_t i=0 ; i<quantity ; ++i)
            _filler_items.push_back(item);
    }

    this->randomize_gold_values(gold_items_count);

    tools::shuffle(_filler_items, _rng);
}

void WorldRandomizer::randomize_gold_values(uint8_t gold_items_count)
{
    constexpr uint16_t AVERAGE_GOLD_PER_CHEST = 35;
    constexpr double MAX_FACTOR_OF_TOTAL_GOLD = 0.16;

    uint16_t total_gold = AVERAGE_GOLD_PER_CHEST * gold_items_count;

    for (uint8_t i = 0; i < gold_items_count ; ++i)
    {
        uint16_t gold_value;

        if (i < gold_items_count - 1)
        {
            double proportion = (double) _rng() / (double) _rng.max();
            double factor = (proportion * MAX_FACTOR_OF_TOTAL_GOLD);

            gold_value = (uint16_t)((double)total_gold * factor);
        }
        else
        {
            gold_value = total_gold;
        }

        if (gold_value == 0)
            gold_value = 1;
        else if (gold_value > 255)
            gold_value = 255;

        total_gold -= gold_value;

        Item* gold_item = _world.add_gold_item(static_cast<uint8_t>(gold_value));
        if(gold_item)
            _filler_items.push_back(gold_item);
    }
}

void WorldRandomizer::init_mandatory_items()
{
    std::map<std::string, uint16_t> mandatory_items_desc;
    if(_options.has_custom_mandatory_items())
    {
        mandatory_items_desc = _options.mandatory_items();
    }
    else
    {
        mandatory_items_desc = {
            {"Magic Sword", 1},      {"Thunder Sword", 1},     {"Sword of Ice", 1},     {"Sword of Gaia", 1},
            {"Steel Breast", 1},     {"Chrome Breast", 1},     {"Shell Breast", 1},     {"Hyper Breast", 1},
            {"Healing Boots", 1},    {"Iron Boots", 1},        {"Fireproof", 1},
            {"Mars Stone", 1},       {"Moon Stone", 1},        {"Saturn Stone", 1},     {"Venus Stone", 1},
            {"Oracle Stone", 1},     {"Statue of Jypta", 1},   {"Spell Book", 1},
        };
    }

    for (auto& [item_name, quantity] : mandatory_items_desc)
    {
        Item* item = _world.item(item_name);
        if(!item)
        {
            std::stringstream msg;
            msg << "Unknown item '" << item_name << "' found in mandatory items.";
            throw RandomizerException(msg.str());
        }
        
        for(uint16_t i=0 ; i<quantity ; ++i)
            _mandatory_items.push_back(item);
    }

    tools::shuffle(_mandatory_items, _rng);
}

void WorldRandomizer::randomize_items()
{
    _solver.setup(_world);

    this->place_mandatory_items();

    bool explored_new_regions = true;
    while(explored_new_regions)
    {
        // Run a solver step to reach a "blocked" state where something needs to be placed in order to continue
        explored_new_regions = _solver.run_until_blocked();
        
        std::vector<ItemSource*> empty_sources = _solver.empty_reachable_item_sources();
        tools::shuffle(empty_sources, _rng);

        this->place_key_items(empty_sources);

        // Fill a fraction of already available sources with filler items
        size_t sources_to_fill_count = (size_t)(empty_sources.size() * _options.filling_rate());
        this->place_filler_items(empty_sources, sources_to_fill_count);

        // Item sources changed, force the solver to update its inventory
        _solver.update_current_inventory();
    }

    // Place the remaining filler items
    std::vector<ItemSource*> empty_sources = _solver.empty_reachable_item_sources();
    this->place_filler_items(empty_sources);

    // Put the end state inside the debug log
    Json& debug_log = _solver.debug_log();
    debug_log["endState"]["unplacedItems"] = Json::array();
    for (Item* item : _filler_items)
        debug_log["endState"]["unplacedItems"].push_back(item->name());
    debug_log["endState"]["remainingSourcesToFill"] = Json::array();
    for(ItemSource* source : empty_sources)
        debug_log["endState"]["remainingSourcesToFill"].push_back(source->name());

    // Analyse items required to complete the seed
    _minimal_items_to_complete = _solver.find_minimal_inventory();
    debug_log["requiredItems"] = Json::array();
    for (Item* item : _minimal_items_to_complete)
        debug_log["requiredItems"].push_back(item->name());
}

static ItemSource* pop_first_compatible_source(std::vector<ItemSource*>& sources, Item* item)
{
    for (uint32_t i = 0; i < sources.size(); ++i)
    {
        if (sources[i]->is_item_compatible(item))
        {
            ItemSource* source = sources[i];
            sources.erase(sources.begin() + i);
            return source;
        }
    }

    return nullptr;
} 

void WorldRandomizer::place_mandatory_items()
{
    Json& debug_log = _solver.debug_log();
    debug_log["steps"]["0"]["comment"] = "Placing mandatory items";

    // Mandatory items are filler items which are always placed first in the randomization, no matter what
    std::vector<ItemSource*> all_empty_item_sources;
    for (ItemSource* source : _world.item_sources())
        if(!source->item())
            all_empty_item_sources.push_back(source);

    tools::shuffle(all_empty_item_sources, _rng);

    for (Item* item : _mandatory_items)
    {
        ItemSource* source = pop_first_compatible_source(all_empty_item_sources, item);
        if(source)
        {
            source->item(item);
            debug_log["steps"]["0"]["placedItems"][source->name()] = item->name();
        }
        else throw NoAppropriateItemSourceException();
    }
}

void WorldRandomizer::place_key_items(std::vector<ItemSource*>& empty_sources)
{
    // List all blocked paths, taking weights into account
    Json& debug_log = _solver.debug_log_for_current_step();
    debug_log["blockedPaths"] = Json::array();

    const std::vector<WorldPath*>& blocked_paths = _solver.blocked_paths();
    std::vector<WorldPath*> weighted_blocked_paths;
    for (WorldPath* path : blocked_paths)
    {
        // If items are not the (only) blocking point for taking this path, let it go
        if(!_solver.missing_regions_to_take_path(path).empty())
            continue;

        debug_log["blockedPaths"].push_back(path->origin()->id() + " --> " + path->destination()->id());
        for(int i=0 ; i<path->weight() ; ++i)
            weighted_blocked_paths.push_back(path);
    }

    if (weighted_blocked_paths.empty())
        return;

    // Randomly choose one of those blocking paths
    tools::shuffle(weighted_blocked_paths, _rng);
    WorldPath* path_to_open = weighted_blocked_paths[0];
    debug_log["chosenPath"].push_back(path_to_open->origin()->id() + " --> " + path_to_open->destination()->id());

    // Place all missing key items for this blocking path
    std::vector<Item*> items_to_place = _solver.missing_items_to_take_path(path_to_open);
    std::vector<Item*> extra_items = path_to_open->items_placed_when_crossing();
    items_to_place.insert(items_to_place.end(), extra_items.begin(), extra_items.end());

    for(Item* item : items_to_place)
    {
        // Place the key item in a compatible source
        ItemSource* compatible_source = pop_first_compatible_source(empty_sources, item);
        if (!compatible_source)
            throw NoAppropriateItemSourceException();

        compatible_source->item(item);
        _logical_playthrough.push_back(compatible_source);
        debug_log["placedKeyItems"][compatible_source->name()] = item->name();
    }
}

void WorldRandomizer::place_filler_items(std::vector<ItemSource*>& empty_sources, size_t count)
{
    count = std::min(count, empty_sources.size());
    count = std::min(count, _filler_items.size());

    Json& debug_log = _solver.debug_log_for_current_step();

    for (size_t i=0 ; i<count ; ++i)
    {
        Item* item = _filler_items[0];
        _filler_items.erase(_filler_items.begin());

        ItemSource* source = pop_first_compatible_source(empty_sources, item);
        if(source)
        {
            source->item(item);
            debug_log["placedFillerItems"][source->name()] = item->name();
        }
        else
            _filler_items.push_back(item);
    }
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

    _world.hint_sources().at("Lithograph")->text(lithograph_hint.str());
}

void WorldRandomizer::randomize_where_is_lithograph_hint()
{
    std::stringstream where_is_litho_hint;
    where_is_litho_hint << "The lithograph will help you finding the jewels. It is "
                        << this->random_hint_for_item(_world.item(ITEM_LITHOGRAPH))
                        << ".";

    _world.hint_sources().at("King Nole's Cave sign")->text(where_is_litho_hint.str());
}

Item* WorldRandomizer::randomize_fortune_teller_hint()
{
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
    _world.hint_sources().at("Mercator fortune teller")->text(fortune_teller_hint.str());

    return hinted_item;
}

Item* WorldRandomizer::randomize_oracle_stone_hint(Item* forbidden_fortune_teller_item)
{
    UnsortedSet<Item*> forbidden_items = {
        forbidden_fortune_teller_item, _world.item(ITEM_RED_JEWEL), _world.item(ITEM_PURPLE_JEWEL),
        _world.item(ITEM_GREEN_JEWEL), _world.item(ITEM_BLUE_JEWEL), _world.item(ITEM_YELLOW_JEWEL)
    };

    // Also excluding items strictly needed to get to Oracle Stone's location
    std::vector<ItemSource*> sources = _world.item_sources_with_item(_world.item(ITEM_ORACLE_STONE));
    WorldRegion* first_source_region = sources.at(0)->region();
    if(first_source_region)
    {
        WorldSolver solver(_world.spawn_region(), first_source_region, _world.starting_inventory());
        std::vector<Item*> min_items_to_reach = solver.find_minimal_inventory();
        for (Item* item : min_items_to_reach)
            forbidden_items.insert(item);
    }

    std::vector<Item*> hintable_items;
    for (Item* item : _minimal_items_to_complete)
    {
        if(!forbidden_items.contains(item))
            hintable_items.push_back(item);
    }
    
    if (!hintable_items.empty())
    {
        tools::shuffle(hintable_items, _rng);
        Item* hinted_item = hintable_items[0];

        std::stringstream oracle_stone_hint;
        oracle_stone_hint << "You will need " << hinted_item->name() << ". It is " << this->random_hint_for_item(hinted_item) << ".";
        _world.hint_sources().at("Oracle Stone")->text(oracle_stone_hint.str());

        return hinted_item;
    }
    
    _world.hint_sources().at("Oracle Stone")->text("The stone looks blurry. It looks like it won't be of any use...");
    return nullptr;
}


void WorldRandomizer::randomize_sign_hints(Item* hinted_fortune_item, Item* hinted_oracle_stone_item)
{
    // A shuffled list of macro regions, used for the "barren / useful region" hints
    UnsortedSet<WorldMacroRegion*> hintable_macro_regions = _world.macro_regions();
    tools::shuffle(hintable_macro_regions, _rng);

    // A shuffled list of potentially optional items, useful for the "this item will be useful / useless" hints
    UnsortedSet<uint8_t> hintable_item_requirements = {
        ITEM_BUYER_CARD,   ITEM_EINSTEIN_WHISTLE,   ITEM_ARMLET,    ITEM_GARLIC, 
        ITEM_IDOL_STONE,   ITEM_CASINO_TICKET,      ITEM_LOGS
    };
    tools::shuffle(hintable_item_requirements, _rng);

    // A shuffled list of items which location is interesting, useful for the "item X is in Y" hints
    UnsortedSet<uint8_t> hintable_item_locations = {
        ITEM_SPIKE_BOOTS,       ITEM_AXE_MAGIC,      ITEM_BUYER_CARD,    ITEM_GARLIC,
        ITEM_EINSTEIN_WHISTLE,  ITEM_ARMLET,         ITEM_IDOL_STONE,
        ITEM_THUNDER_SWORD,     ITEM_HEALING_BOOTS,  ITEM_VENUS_STONE,   ITEM_STATUE_JYPTA,
        ITEM_SUN_STONE,         ITEM_KEY,            ITEM_SAFETY_PASS,   ITEM_LOGS,
        ITEM_GOLA_EYE,          ITEM_GOLA_NAIL,      ITEM_GOLA_FANG,     ITEM_GOLA_HORN
    };
    tools::shuffle(hintable_item_locations, _rng);

    // Remove items that have been already hinted by special hints
    if(hinted_fortune_item)
        hintable_item_locations.erase(hinted_fortune_item->id());
    if(hinted_oracle_stone_item)
        hintable_item_locations.erase(hinted_oracle_stone_item->id());

    for (auto& [k, hint_source] : _world.hint_sources())
    {
        // Hint source is special (e.g. Oracle Stone, Lithograph...), don't handle it here
        if(hint_source->special())
            continue;

        WorldSolver solver(_world.spawn_region(), hint_source->region(), _world.starting_inventory());
        UnsortedSet<Item*> min_inventory_at_sign = solver.find_minimal_inventory();
        
        double randomNumber = (double) _rng() / (double) _rng.max();

        ////////////////////////////////////////////////////////////////
        // "Barren / pleasant surprise" (30%)
        if (randomNumber < 0.3 && !hintable_macro_regions.empty())
        {
            WorldMacroRegion* macroRegion = hintable_macro_regions[0];
            hintable_macro_regions.erase(macroRegion);

            if (_world.is_macro_region_avoidable(macroRegion))
                hint_source->text("What you are looking for is not in " + macroRegion->name() + ".");
            else
                hint_source->text("You might have a pleasant surprise wandering in " + macroRegion->name() + ".");

            continue;
        }

        ////////////////////////////////////////////////////////////////
        // "You will / won't need {item} to finish" (25%)
        if (randomNumber < 0.55)
        {
            Item* hinted_item_requirement = nullptr;
            for(uint8_t item_id : hintable_item_requirements)
            {
                Item* tested_item = _world.item(item_id);
                if(!min_inventory_at_sign.contains(tested_item))
                {
                    // If item was not already obtained at sign, we can hint it
                    hinted_item_requirement = tested_item;
                    hintable_item_requirements.erase(item_id);
                    break;
                }
            }

            if(hinted_item_requirement)
            {
                if (!_world.is_item_avoidable(hinted_item_requirement))
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
                if(!min_inventory_at_sign.contains(tested_item))
                {
                    // If item was not already obtained at sign, we can hint it
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
    const std::vector<std::string>& region_hints = itemSource->region()->hints();
    const std::vector<std::string>& source_hints = itemSource->hints();
    
    std::vector<std::string> all_hints;
    all_hints.insert(all_hints.end(), region_hints.begin(), region_hints.end());
    all_hints.insert(all_hints.end(), source_hints.begin(), source_hints.end());
    
    if(all_hints.empty())
        return "in an unknown place";

    tools::shuffle(all_hints, _rng);
    return all_hints[0];
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
