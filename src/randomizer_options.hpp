#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <chrono>
#include <fstream>

#include <landstalker_lib/tools/json.hpp>
#include <landstalker_lib/tools/tools.hpp>
#include <landstalker_lib/tools/argument_dictionary.hpp>

class RandomizerOptions 
{
private:
    static constexpr uint8_t STARTING_LIFE_USE_SPAWN_LOCATION_VALUE = 0;

    // ------------- Game patching settings -------------
    // (included in permalink, presets & plandos)
    uint8_t _jewel_count = 2;
    bool _use_armor_upgrades = true;
    uint8_t _startingLife = STARTING_LIFE_USE_SPAWN_LOCATION_VALUE;
    uint16_t _startingGold = 0;
    std::map<std::string, uint8_t> _starting_items;
    bool _fix_armlet_skip = true;
    bool _fix_tree_cutting_glitch = true;
    bool _consumable_record_book = false;
    bool _remove_gumi_boulder = false;
    bool _remove_tibor_requirement = false;
    bool _all_trees_visited_at_start = false;
    uint16_t _enemies_damage_factor = 100;
    uint16_t _enemies_health_factor = 100;
    uint16_t _enemies_armor_factor = 100;
    uint16_t _enemies_golds_factor = 100;
    uint16_t _enemies_drop_chance_factor = 100;
    uint8_t _health_gained_per_lifestock = 1;

    // ------------- Randomization settings -------------
    // (included in permalink & presets, not in plandos)
    uint32_t _seed = 0;
    bool _allow_spoiler_log = true;
    std::vector<std::string> _possible_spawn_locations;
    bool _shuffle_tibor_trees = false;
    bool _ghost_jumping_in_logic = false;
    bool _damage_boosting_in_logic = false;
    std::map<uint8_t, uint16_t> _items_distribution;
    uint8_t _hints_count = 10;

    // ------------- Plando world JSON -------------
    Json _world_json;

    // ------------- Model patch -------------
    Json _model_patch_items;
    Json _model_patch_spawns;

    // ------------- Personal settings -------------
    // (not included in permalink nor presets)
    bool _add_ingame_item_tracker = false;
    uint16_t _hud_color = 0x824;
    std::pair<uint16_t, uint16_t> _nigel_colors = std::make_pair(0x08A0, 0x0240);

public:
    RandomizerOptions() = default;
    explicit RandomizerOptions(const ArgumentDictionary& args);
    
    void parse_permalink(const std::string& permalink);
    void parse_arguments(const ArgumentDictionary& args);
    void parse_personal_settings(const ArgumentDictionary& args);
    void validate() const;

    [[nodiscard]] Json to_json() const;
    void parse_json(const Json& json);

    // Game patching options 
    [[nodiscard]] const std::vector<std::string>& possible_spawn_locations() const { return _possible_spawn_locations; }
    [[nodiscard]] uint8_t jewel_count() const { return _jewel_count; }
    [[nodiscard]] bool use_armor_upgrades() const { return _use_armor_upgrades; }
    [[nodiscard]] uint8_t starting_life() const { return _startingLife; }
    [[nodiscard]] uint16_t starting_gold() const { return _startingGold; }
    [[nodiscard]] const std::map<std::string, uint8_t>& starting_items() const { return _starting_items; }
    [[nodiscard]] bool fix_armlet_skip() const { return _fix_armlet_skip; }
    [[nodiscard]] bool fix_tree_cutting_glitch() const { return _fix_tree_cutting_glitch; }
    [[nodiscard]] bool consumable_record_book() const { return _consumable_record_book; }
    [[nodiscard]] bool remove_gumi_boulder() const { return _remove_gumi_boulder; }
    [[nodiscard]] bool remove_tibor_requirement() const { return _remove_tibor_requirement; }
    [[nodiscard]] bool all_trees_visited_at_start() const { return _all_trees_visited_at_start; }
    [[nodiscard]] double enemies_damage_factor() const { return (double)_enemies_damage_factor / 100.0; }
    [[nodiscard]] double enemies_health_factor() const { return (double)_enemies_health_factor / 100.0; }
    [[nodiscard]] double enemies_armor_factor() const { return (double)_enemies_armor_factor / 100.0; }
    [[nodiscard]] double enemies_golds_factor() const { return (double)_enemies_golds_factor / 100.0; }
    [[nodiscard]] double enemies_drop_chance_factor() const { return (double)_enemies_drop_chance_factor / 100.0; }
    [[nodiscard]] uint8_t health_gained_per_lifestock() const { return _health_gained_per_lifestock; }

    // Randomization options
    [[nodiscard]] uint32_t seed() const { return _seed; }
    [[nodiscard]] bool allow_spoiler_log() const { return _allow_spoiler_log; }
    [[nodiscard]] bool shuffle_tibor_trees() const { return _shuffle_tibor_trees; }
    [[nodiscard]] bool handle_ghost_jumping_in_logic() const { return _ghost_jumping_in_logic; }
    [[nodiscard]] bool handle_damage_boosting_in_logic() const { return _damage_boosting_in_logic; }
    [[nodiscard]] const std::map<uint8_t, uint16_t>& items_distribution() const { return _items_distribution; }
    [[nodiscard]] uint8_t hints_count() const { return _hints_count; }

    // Plando-specific options
    [[nodiscard]] const Json& world_json() const { return _world_json; }

    // Model patch
    [[nodiscard]] const Json& items_model_patch() const { return _model_patch_items; }
    [[nodiscard]] const Json& spawn_locations_model_patch() const { return _model_patch_spawns; }

    // Personal options 
    [[nodiscard]] bool add_ingame_item_tracker() const { return _add_ingame_item_tracker; }
    [[nodiscard]] uint16_t hud_color() const { return _hud_color; }
    [[nodiscard]] const std::pair<uint16_t, uint16_t>& nigel_colors() const { return _nigel_colors; }

    [[nodiscard]] std::vector<std::string> hash_words() const;
    [[nodiscard]] std::string hash_sentence() const { return tools::join(this->hash_words(), " "); }
    [[nodiscard]] std::string permalink() const;
};
