#pragma once

#include <vector>
#include <string>
#include <random>

#include <landstalker_lib/tools/unsorted_set.hpp>
#include <landstalker_lib/model/world.hpp>

#include "randomizer_options.hpp"
#include "world_solver.hpp"
#include "logic_model/world_logic.hpp"


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

    std::vector<Item*> _item_pool;
    std::map<uint8_t, uint16_t> _item_pool_quantities;

    std::vector<Item*> _minimal_items_to_complete;
    std::vector<ItemSource*> _logical_playthrough;

public:
    WorldRandomizer(World& world, WorldLogic& logic, const RandomizerOptions& options);
    
    void randomize();

    [[nodiscard]] bool is_region_avoidable(WorldRegion* region) const;
    [[nodiscard]] bool is_item_avoidable(Item* item) const;

    [[nodiscard]] Json playthrough_as_json() const;
    [[nodiscard]] Json& debug_log_as_json() { return _solver.debug_log(); }

private:
    // First pass randomizations (before items)
    void randomize_spawn_location();
    void randomize_dark_rooms();
    void randomize_tibor_trees();
    void randomize_fahl_enemies();

    // Item randomization
    void randomize_items();
    void init_item_pool();
    [[nodiscard]] Item* generate_gold_item();
    ItemSource* place_item_randomly(Item* item, std::vector<ItemSource*> possible_sources);
    Item* fill_item_source_randomly(ItemSource* source);
    [[nodiscard]] bool test_item_source_compatibility(ItemSource* source, Item* item) const;
    [[nodiscard]] std::vector<WorldPath*> build_weighted_blocked_paths_list();
    void open_random_blocked_path();
    void place_remaining_items();

    // Hints randomization
    void randomize_hints();
    void randomize_lithograph_hint();
    void randomize_where_is_lithograph_hint();
    [[nodiscard]] Item* randomize_fortune_teller_hint();
    [[nodiscard]] Item* randomize_oracle_stone_hint(Item* fortune_hinted_item);
    void randomize_sign_hints(Item* fortune_hinted_item, Item* oracle_stone_hinted_item);

    [[nodiscard]] std::string random_hint_for_item(Item* item);
    [[nodiscard]] std::string random_hint_for_item_source(ItemSource* source);
};
