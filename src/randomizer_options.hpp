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
public:
    RandomizerOptions();
    RandomizerOptions(const ArgumentDictionary& args);
    ~RandomizerOptions();
    
    void parse_permalink(const std::string& permalink);
    void parse_arguments(const ArgumentDictionary& args);
    void parse_personal_settings(const ArgumentDictionary& args);
    void validate();

    Json to_json() const;
    void parse_json(const Json& json);
    Json personal_settings_as_json() const;

    // Game patching options 
    const std::vector<std::string>& possible_spawn_locations() const { return _possible_spawn_locations; }
    uint8_t jewel_count() const { return _jewel_count; }
    bool use_armor_upgrades() const { return _use_armor_upgrades; }
    uint8_t starting_life() const { return _startingLife; }
    uint16_t starting_gold() const { return _startingGold; }
    const std::map<std::string, uint8_t>& starting_items() const { return _starting_items; }
    bool fix_armlet_skip() const { return _fix_armlet_skip; }
    bool fix_tree_cutting_glitch() const { return _fix_tree_cutting_glitch; }
    bool consumable_record_book() const { return _consumable_record_book; }
    bool remove_gumi_boulder() const { return _remove_gumi_boulder; }
    bool remove_tibor_requirement() const { return _remove_tibor_requirement; }
    bool all_trees_visited_at_start() const { return _all_trees_visited_at_start; }
    double enemies_damage_factor() const { return (double)_enemies_damage_factor / 100.0; }
    double enemies_health_factor() const { return (double)_enemies_health_factor / 100.0; }
    double enemies_armor_factor() const { return (double)_enemies_armor_factor / 100.0; }
    double enemies_golds_factor() const { return (double)_enemies_golds_factor / 100.0; }
    double enemies_drop_chance_factor() const { return (double)_enemies_drop_chance_factor / 100.0; }

    // Randomization options
    uint32_t seed() const { return _seed; }
    bool allow_spoiler_log() const { return _allow_spoiler_log; }
    uint16_t item_sources_window() const { return _item_sources_window; }
    bool shuffle_tibor_trees() const { return _shuffle_tibor_trees; }
    bool handle_ghost_jumping_in_logic() const { return _ghost_jumping_in_logic; }
    bool handle_damage_boosting_in_logic() const { return _damage_boosting_in_logic; }
    bool has_custom_mandatory_items() const { return _mandatory_items != nullptr; }
    const std::map<std::string, uint16_t>& mandatory_items() const { return *_mandatory_items; }
    bool has_custom_filler_items() const { return _filler_items != nullptr; }
    const std::map<std::string, uint16_t>& filler_items() const { return *_filler_items; }

    // Model patch
    const Json& items_model_patch() const { return _model_patch_items; }
    const Json& spawn_locations_model_patch() const { return _model_patch_spawns; }

    // Personal options 
    bool add_ingame_item_tracker() const { return _add_ingame_item_tracker; }
    uint16_t hud_color() const { return _hud_color; }

    std::vector<std::string> hash_words() const;
    std::string hash_sentence() const { return tools::join(this->hash_words(), " "); }
    std::string permalink() const;

    // Plando-specific options
    bool is_plando() const { return _plando_enabled; }
    const Json& input_plando_json() const { return _plando_json; }

private:
    // ------------- Game patching settings -------------
    // (included in permalink, presets & plandos)
    uint8_t _jewel_count;
    bool _use_armor_upgrades;
    uint8_t _startingLife;
    uint16_t _startingGold;
    std::map<std::string, uint8_t> _starting_items;
    bool _fix_armlet_skip;
    bool _fix_tree_cutting_glitch;
    bool _consumable_record_book;
    bool _remove_gumi_boulder;
    bool _remove_tibor_requirement;
    bool _all_trees_visited_at_start;
    uint16_t _enemies_damage_factor;
    uint16_t _enemies_health_factor;
    uint16_t _enemies_armor_factor;
    uint16_t _enemies_golds_factor;
    uint16_t _enemies_drop_chance_factor;

    // ------------- Randomization settings -------------
    // (included in permalink & presets, not in plandos)
    uint32_t _seed;
    bool _allow_spoiler_log;
    uint16_t _item_sources_window;
    std::vector<std::string> _possible_spawn_locations;
    bool _shuffle_tibor_trees;
    bool _ghost_jumping_in_logic;
    bool _damage_boosting_in_logic;
    std::map<std::string, uint16_t>* _mandatory_items;
    std::map<std::string, uint16_t>* _filler_items;

    // ------------- Model patch -------------
    Json _model_patch_items;
    Json _model_patch_spawns;

    // ------------- Personal settings -------------
    // (not included in permalink nor presets)
    bool _add_ingame_item_tracker;
    uint16_t _hud_color;

    // Plando-specific arguments
    bool _plando_enabled;
    Json _plando_json;
};
