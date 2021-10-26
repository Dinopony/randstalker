#pragma once

#include <vector>
#include <string>
#include <random>

#include "model/item.hpp"
#include "model/item_source.hpp"
#include "model/world_region.hpp"

#include "tools/unsorted_set.hpp"

#include "randomizer_options.hpp"
#include "world.hpp"


class WorldRandomizer
{
private:
    World& _world;
    const RandomizerOptions& _options;
    Json _debug_log_json;
    std::mt19937 _rng;

    std::vector<Item*> _filler_items;
    std::vector<Item*> _mandatoryItems;
    uint8_t _gold_items_count;

    UnsortedSet<WorldRegion*> _regions_to_explore;
    UnsortedSet<WorldRegion*> _explored_regions;
    std::vector<ItemSource*> _item_sources_to_fill;
    std::vector<Item*> _inventory;
    std::vector<WorldPath*> _pending_paths;
    std::vector<Item*> _minimal_items_to_complete;
    std::vector<ItemSource*> _logical_playthrough;

public:
    WorldRandomizer(World& world, const RandomizerOptions& options);

    void randomize();

    Json playthrough_as_json() const;
    const Json& debug_log_as_json() const { return _debug_log_json; }

private:
    void init_filler_items();
    void init_mandatory_items();
    void init_inventory();
    
    // First pass randomizations (before items)
    void randomize_spawn_location();
    void randomize_gold_values();
    void randomize_dark_rooms();

    // Second pass randomizations (items)
    void place_mandatory_items();
    void randomize_items();

    void place_filler_items_phase(Json& debugLogStepJson, size_t count, Item* lastResortFiller = nullptr);
    void exploration_phase(Json& debugLogStepJson);
    void place_key_items_phase(Json& debugLogStepJson);
    void unlock_phase();

    // Third pass randomizations (after items)
    void randomize_hints();
    void randomize_lithograph_hint();
    void randomize_where_is_lithograph_hint();
    Item* randomize_fortune_teller_hint();
    Item* randomize_oracle_stone_hint(Item* forbiddenFortuneTellerItem);
    void randomize_sign_hints(Item* hintedFortuneItem, Item* hintedOracleStoneItem);

    std::string random_hint_for_item(Item* item);
    std::string random_hint_for_item_source(ItemSource* itemSource);

    void randomize_tibor_trees();
};
