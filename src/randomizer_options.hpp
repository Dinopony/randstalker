#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <chrono>
#include <fstream>

#include "extlibs/json.hpp"

#include "tools/tools.hpp"
#include "tools/argument_dictionary.hpp"

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
    bool dungeon_sign_hints() const { return _dungeonSignHints; }
    uint8_t starting_life() const { return _startingLife; }
    uint16_t starting_gold() const { return _startingGold; }
    const std::map<std::string, uint8_t>& starting_items() const { return _starting_items; }
    const std::map<std::string, uint16_t>& item_prices() const { return _itemPrices; }
    bool fix_armlet_skip() const { return _fix_armlet_skip; }
    bool fix_tree_cutting_glitch() const { return _fix_tree_cutting_glitch; }
    const std::map<std::string, uint8_t>& item_max_quantities() const { return _item_max_quantities; }
    bool consumable_record_book() const { return _consumable_record_book; }

    // Randomization options
    uint32_t seed() const { return _seed; }
    bool allow_spoiler_log() const { return _allow_spoiler_log; }
    double filling_rate() const { return _fillingRate; }
    bool shuffle_tibor_trees() const { return _shuffle_tibor_trees; }
    bool handle_ghost_jumping_in_logic() const { return _ghost_jumping_in_logic; }
    bool has_custom_mandatory_items() const { return _mandatory_items != nullptr; }
    const std::map<std::string, uint16_t>& mandatory_items() const { return *_mandatory_items; }
    bool has_custom_filler_items() const { return _filler_items != nullptr; }
    const std::map<std::string, uint16_t>& filler_items() const { return *_filler_items; }

    std::vector<std::string> hash_words() const;
    std::string hash_sentence() const { return Tools::join(this->hash_words(), " "); }
    std::string permalink() const;

    // Personal options 
    bool add_ingame_item_tracker() const { return _add_ingame_item_tracker; }
    const std::string hud_color() const { return _hud_color; }

    // Plando-specific options
    bool is_plando() const { return _plando_enabled; }
    const Json& input_plando_json() const { return _plando_json; }

private:
    // ------------- Game patching settings -------------
    // (included in permalink, presets & plandos)
    uint32_t _jewel_count;
    bool _use_armor_upgrades;
    bool _dungeonSignHints;
    uint8_t _startingLife;
    uint16_t _startingGold;
    std::map<std::string, uint8_t> _starting_items;
    std::map<std::string, uint16_t> _itemPrices;
    bool _fix_armlet_skip;
    bool _fix_tree_cutting_glitch;
    std::map<std::string, uint8_t> _item_max_quantities;
    bool _consumable_record_book;
    
    // ------------- Randomization settings -------------
    // (included in permalink & presets, not in plandos)
    uint32_t _seed;
    bool _allow_spoiler_log;
    double _fillingRate;
    std::vector<std::string> _possible_spawn_locations;
    bool _shuffle_tibor_trees;
    bool _ghost_jumping_in_logic;
    std::map<std::string, uint16_t>* _mandatory_items;
    std::map<std::string, uint16_t>* _filler_items;

    // ------------- Personal settings -------------
    // (not included in permalink nor presets)
    bool _add_ingame_item_tracker;
    std::string _hud_color;

    // Plando-specific arguments
    bool _plando_enabled;
    Json _plando_json;
};
