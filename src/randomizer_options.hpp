#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <chrono>
#include <fstream>

#include <landstalker_lib/tools/json.hpp>
#include <landstalker_lib/constants/values.hpp>
#include <landstalker_lib/tools/stringtools.hpp>
#include <landstalker_lib/tools/argument_dictionary.hpp>

class Item;

class RandomizerOptions 
{
private:
    static constexpr uint8_t STARTING_LIFE_USE_SPAWN_LOCATION_VALUE = 0;

    std::array<std::string, ITEM_COUNT+1> _item_names;

    // ------------- Game patching settings -------------
    // (included in permalink, presets & plandos)
    uint8_t _jewel_count = 2;
    bool _use_armor_upgrades = true;
    uint8_t _starting_life = STARTING_LIFE_USE_SPAWN_LOCATION_VALUE;
    uint16_t _starting_gold = 0;
    std::array<uint8_t, ITEM_COUNT> _starting_items {};
    bool _fix_armlet_skip = true;
    bool _remove_tree_cutting_glitch_drops = true;
    bool _consumable_record_book = false;
    bool _consumable_spell_book = false;
    bool _remove_gumi_boulder = false;
    bool _remove_tibor_requirement = false;
    bool _all_trees_visited_at_start = false;
    uint16_t _enemies_damage_factor = 100;
    uint16_t _enemies_health_factor = 100;
    uint16_t _enemies_armor_factor = 100;
    uint16_t _enemies_golds_factor = 100;
    uint16_t _enemies_drop_chance_factor = 100;
    uint8_t _health_gained_per_lifestock = 1;
    bool _fast_transitions = true;

    // ------------- Randomization settings -------------
    // (included in permalink & presets, not in plandos)
    uint32_t _seed = 0;
    bool _allow_spoiler_log = true;
    std::vector<std::string> _possible_spawn_locations;
    bool _shuffle_tibor_trees = false;
    bool _enemy_jumping_in_logic = false;
    bool _damage_boosting_in_logic = false;
    bool _tree_cutting_glitch_in_logic = false;
    bool _allow_whistle_usage_behind_trees = false;
    std::array<uint8_t, ITEM_COUNT+1> _items_distribution {};
    uint16_t _hints_distribution_region_requirement = 0;
    uint16_t _hints_distribution_item_requirement = 0;
    uint16_t _hints_distribution_item_location = 0;
    uint16_t _hints_distribution_dark_region = 0;
    uint16_t _hints_distribution_joke = 0;

    // ------------- Events -------------
    bool _christmas_event = false;

    // ------------- Plando world JSON -------------
    Json _world_json;

    // ------------- Model patch -------------
    Json _model_patch_items;
    Json _model_patch_spawns;
    Json _model_patch_hint_sources;

public:
    explicit RandomizerOptions(const ArgumentDictionary& args, const std::array<std::string, ITEM_COUNT+1>& item_names);

    [[nodiscard]] Json to_json() const;
    void parse_json(const Json& json);

    // Game patching options
    [[nodiscard]] uint8_t jewel_count() const { return _jewel_count; }
    [[nodiscard]] bool use_armor_upgrades() const { return _use_armor_upgrades; }
    [[nodiscard]] uint8_t starting_life() const { return _starting_life; }
    [[nodiscard]] uint16_t starting_gold() const { return _starting_gold; }
    [[nodiscard]] const std::array<uint8_t, ITEM_COUNT>& starting_items() const { return _starting_items; }
    [[nodiscard]] bool fix_armlet_skip() const { return _fix_armlet_skip; }
    [[nodiscard]] bool remove_tree_cutting_glitch_drops() const { return _remove_tree_cutting_glitch_drops; }
    [[nodiscard]] bool consumable_record_book() const { return _consumable_record_book; }
    [[nodiscard]] bool consumable_spell_book() const { return _consumable_spell_book; }
    [[nodiscard]] bool remove_gumi_boulder() const { return _remove_gumi_boulder; }
    [[nodiscard]] bool remove_tibor_requirement() const { return _remove_tibor_requirement; }
    [[nodiscard]] bool all_trees_visited_at_start() const { return _all_trees_visited_at_start; }
    [[nodiscard]] double enemies_damage_factor() const { return (double)_enemies_damage_factor / 100.0; }
    [[nodiscard]] double enemies_health_factor() const { return (double)_enemies_health_factor / 100.0; }
    [[nodiscard]] double enemies_armor_factor() const { return (double)_enemies_armor_factor / 100.0; }
    [[nodiscard]] double enemies_golds_factor() const { return (double)_enemies_golds_factor / 100.0; }
    [[nodiscard]] double enemies_drop_chance_factor() const { return (double)_enemies_drop_chance_factor / 100.0; }
    [[nodiscard]] uint8_t health_gained_per_lifestock() const { return _health_gained_per_lifestock; }
    [[nodiscard]] bool fast_transitions() const { return _fast_transitions; }

    // Randomization options
    [[nodiscard]] uint32_t seed() const { return _seed; }
    [[nodiscard]] const std::vector<std::string>& possible_spawn_locations() const { return _possible_spawn_locations; }
    [[nodiscard]] bool allow_spoiler_log() const { return _allow_spoiler_log; }
    [[nodiscard]] bool shuffle_tibor_trees() const { return _shuffle_tibor_trees; }
    [[nodiscard]] bool handle_enemy_jumping_in_logic() const { return _enemy_jumping_in_logic; }
    [[nodiscard]] bool handle_damage_boosting_in_logic() const { return _damage_boosting_in_logic; }
    [[nodiscard]] bool handle_tree_cutting_glitch_in_logic() const { return _tree_cutting_glitch_in_logic; }
    [[nodiscard]] bool allow_whistle_usage_behind_trees() const { return _allow_whistle_usage_behind_trees; }
    [[nodiscard]] const std::array<uint8_t, ITEM_COUNT+1>& items_distribution() const { return _items_distribution; }

    [[nodiscard]] uint16_t hints_count() const { return _hints_distribution_region_requirement
                                                     + _hints_distribution_item_requirement
                                                     + _hints_distribution_item_location
                                                     + _hints_distribution_dark_region
                                                     + _hints_distribution_joke; }
    [[nodiscard]] uint16_t hints_distribution_region_requirement() const { return _hints_distribution_region_requirement; }
    [[nodiscard]] uint16_t hints_distribution_item_requirement() const { return _hints_distribution_item_requirement; }
    [[nodiscard]] uint16_t hints_distribution_item_location() const { return _hints_distribution_item_location; }
    [[nodiscard]] uint16_t hints_distribution_dark_region() const { return _hints_distribution_dark_region; }
    [[nodiscard]] uint16_t hints_distribution_joke() const { return _hints_distribution_joke; }

    // Event
    [[nodiscard]] bool christmas_event() const { return _christmas_event; }

    // Plando-specific options
    [[nodiscard]] const Json& world_json() const { return _world_json; }

    // Model patch
    [[nodiscard]] const Json& items_model_patch() const { return _model_patch_items; }
    [[nodiscard]] const Json& spawn_locations_model_patch() const { return _model_patch_spawns; }
    [[nodiscard]] const Json& hint_sources_model_patch() const { return _model_patch_hint_sources; }

    [[nodiscard]] std::vector<std::string> hash_words() const;
    [[nodiscard]] std::string hash_sentence() const { return stringtools::join(this->hash_words(), " "); }
    [[nodiscard]] std::string permalink() const;

private:
    void parse_permalink(std::string permalink);
    void parse_arguments(const ArgumentDictionary& args);
    void validate() const;
};
