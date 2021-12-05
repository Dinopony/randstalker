#pragma once

#include <vector>
#include <string>
#include <random>

#include "tools/unsorted_set.hpp"

#include "randomizer_options.hpp"
#include "world_solver.hpp"

#include "logic_model/world_logic.hpp"
#include "world_model/world.hpp"

class Item;
class ItemSource;
class HintSource;
class SpawnLocation;

class WorldRandomizer
{
private:
    World& _world;
    WorldLogic& _logic;
    WorldSolver _solver;
    const RandomizerOptions& _options;
    std::mt19937 _rng;

    std::vector<Item*> _filler_items;
    std::vector<Item*> _mandatory_items;

    std::vector<Item*> _minimal_items_to_complete;
    std::vector<ItemSource*> _logical_playthrough;

    std::map<std::string, HintSource*> _hint_sources;
    std::map<std::string, SpawnLocation*> _spawn_locations;
    const WorldRegion* _dark_region;

public:
    WorldRandomizer(World& world, WorldLogic& logic, const RandomizerOptions& options);
    ~WorldRandomizer();
    
    void randomize();

    const WorldRegion* dark_region() const { return _dark_region; }

    bool is_region_avoidable(WorldRegion* region) const;
    bool is_item_avoidable(Item* item) const;

    Json playthrough_as_json() const;
    Json& debug_log_as_json() { return _solver.debug_log(); }

    const std::map<std::string, SpawnLocation*>& spawn_locations() const { return _spawn_locations; }
    void add_spawn_location(SpawnLocation* spawn);

    const std::map<std::string, HintSource*>& hint_sources() const { return _hint_sources; }

private:
    void init_spawn_locations();
    void init_hint_sources();

    void init_filler_items();
    void init_mandatory_items();
    
    // First pass randomizations (before items)
    void randomize_spawn_location();
    void randomize_gold_values(uint8_t gold_items_count);
    void randomize_dark_rooms();
    void randomize_tibor_trees();
    void randomize_fahl_enemies();

    // Item randomization
    void randomize_items();
    void place_mandatory_items();
    void place_key_items(std::vector<ItemSource*>& empty_sources);
    void place_filler_items(std::vector<ItemSource*>& empty_sources, size_t count = SIZE_MAX);

    // Hints randomization
    void randomize_hints();
    void randomize_lithograph_hint();
    void randomize_where_is_lithograph_hint();
    Item* randomize_fortune_teller_hint();
    Item* randomize_oracle_stone_hint(Item* fortune_hinted_item);
    void randomize_sign_hints(Item* fortune_hinted_item, Item* oracle_stone_hinted_item);

    std::string random_hint_for_item(Item* item);
    std::string random_hint_for_item_source(ItemSource* source);
};
