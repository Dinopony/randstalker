#include "randomizer_options.hpp"

#include <iostream>

#include <landstalker_lib/constants/item_codes.hpp>
#include <landstalker_lib/tools/stringtools.hpp>
#include <landstalker_lib/tools/vectools.hpp>
#include <landstalker_lib/tools/bitstream_writer.hpp>
#include <landstalker_lib/tools/bitstream_reader.hpp>
#include <landstalker_lib/exceptions.hpp>

#include "tools/base64.hpp"

RandomizerOptions::RandomizerOptions(const ArgumentDictionary& args, const std::array<std::string, ITEM_COUNT+1>& item_names)
{
    _item_names = item_names;
    _starting_items.fill(0);
    _items_distribution.fill(0);

    std::string permalink_string = args.get_string("permalink");
    if(!permalink_string.empty())
    {
        this->parse_permalink(permalink_string);
    }
    else
    {
        // Parse seed from args, or generate a random one if it's missing
        std::string seed_string = args.get_string("seed", "random");
        try {
            _seed = (uint32_t) std::stoul(seed_string);
        } catch (std::invalid_argument&) {
            _seed = (uint32_t) std::chrono::system_clock::now().time_since_epoch().count();
        }

        std::string preset_path = args.get_string("preset");
        if(!preset_path.empty())
        {
            std::ifstream preset_file(preset_path);
            if(!preset_file)
                throw LandstalkerException("Could not open preset file at given path '" + preset_path + "'");

            std::cout << "Reading preset file '" << preset_path << "'...\n\n";

            Json preset_json;
            preset_file >> preset_json;
            this->parse_json(preset_json);
        }

        this->parse_arguments(args);
    }

    this->validate();
}

void RandomizerOptions::parse_arguments(const ArgumentDictionary& args)
{
    if(args.contains("spawnlocation"))
    {
        std::string spawn_name = args.get_string("spawnlocation");
        stringtools::to_lower(spawn_name);
        if(spawn_name == "random")
            _possible_spawn_locations = {};
        else
            _possible_spawn_locations = { spawn_name };
    }

    if(args.contains("jewelcount"))           _jewel_count = args.get_integer("jewelcount");
    if(args.contains("armorupgrades"))        _use_armor_upgrades = args.get_boolean("armorupgrades");
    if(args.contains("norecordbook"))         _starting_items[ITEM_RECORD_BOOK] = 0;
    if(args.contains("nospellbook"))          _starting_items[ITEM_SPELL_BOOK] = 0;
    if(args.contains("startinglife"))         _starting_life = args.get_integer("startinglife");
    if(args.contains("shuffletrees"))         _shuffle_tibor_trees = args.get_boolean("shuffletrees");
    if(args.contains("allowspoilerlog"))      _allow_spoiler_log = args.get_boolean("allowspoilerlog");
}

Json RandomizerOptions::to_json() const
{
    Json json;

    // Game settings 
    json["gameSettings"]["jewelCount"] = _jewel_count;
    json["gameSettings"]["armorUpgrades"] = _use_armor_upgrades;
    json["gameSettings"]["startingGold"] = _starting_gold;
    json["gameSettings"]["startingLife"] = _starting_life;

    json["gameSettings"]["startingItems"] = Json::object();
    for(uint8_t i=0 ; i<ITEM_COUNT ; ++i)
    {
        if(_starting_items[i] > 0)
            json["gameSettings"]["startingItems"][_item_names[i]] = _starting_items[i];
    }

    json["gameSettings"]["fixArmletSkip"] = _fix_armlet_skip;
    json["gameSettings"]["removeTreeCuttingGlitchDrops"] = _remove_tree_cutting_glitch_drops;
    json["gameSettings"]["consumableRecordBook"] = _consumable_record_book;
    json["gameSettings"]["consumableSpellBook"] = _consumable_spell_book;
    json["gameSettings"]["removeGumiBoulder"] = _remove_gumi_boulder;
    json["gameSettings"]["removeTiborRequirement"] = _remove_tibor_requirement;
    json["gameSettings"]["allTreesVisitedAtStart"] = _all_trees_visited_at_start;
    json["gameSettings"]["enemiesDamageFactor"] = _enemies_damage_factor;
    json["gameSettings"]["enemiesHealthFactor"] = _enemies_health_factor;
    json["gameSettings"]["enemiesArmorFactor"] = _enemies_armor_factor;
    json["gameSettings"]["enemiesGoldsFactor"] = _enemies_golds_factor;
    json["gameSettings"]["enemiesDropChanceFactor"] = _enemies_drop_chance_factor;
    json["gameSettings"]["healthGainedPerLifestock"] = _health_gained_per_lifestock;
    json["gameSettings"]["fastMenuTransitions"] = _fast_menu_transitions;

    // Randomizer settings
    json["randomizerSettings"]["allowSpoilerLog"] = _allow_spoiler_log;
    json["randomizerSettings"]["spawnLocations"] = _possible_spawn_locations;
    json["randomizerSettings"]["shuffleTrees"] = _shuffle_tibor_trees;
    json["randomizerSettings"]["enemyJumpingInLogic"] = _enemy_jumping_in_logic;
    json["randomizerSettings"]["damageBoostingInLogic"] = _damage_boosting_in_logic;
    json["randomizerSettings"]["treeCuttingGlitchInLogic"] = _tree_cutting_glitch_in_logic;
    json["randomizerSettings"]["allowWhistleUsageBehindTrees"] = _allow_whistle_usage_behind_trees;
    json["randomizerSettings"]["hintsDistribution"] = {
        { "regionRequirement", _hints_distribution_region_requirement },
        { "itemRequirement", _hints_distribution_item_requirement },
        { "itemLocation", _hints_distribution_item_location },
        { "darkRegion", _hints_distribution_dark_region },
        { "joke", _hints_distribution_joke }
    };

    if(!_model_patch_items.empty())
        json["modelPatch"]["items"] = _model_patch_items;
    if(!_model_patch_spawns.empty())
        json["modelPatch"]["spawnLocations"] = _model_patch_spawns;
    if(!_model_patch_hint_sources.empty())
        json["modelPatch"]["hintSources"] = _model_patch_hint_sources;

    return json;
}

void RandomizerOptions::parse_json(const Json& json)
{
    if(json.contains("permalink"))
    {
        this->parse_permalink(json.at("permalink"));
        return;
    }

    if(json.contains("gameSettings"))
    {
        const Json& game_settings_json = json.at("gameSettings");

        if(game_settings_json.contains("jewelCount"))            
            _jewel_count = game_settings_json.at("jewelCount");
        if(game_settings_json.contains("armorUpgrades"))
            _use_armor_upgrades = game_settings_json.at("armorUpgrades");
        if(game_settings_json.contains("startingLife"))
            _starting_life = game_settings_json.at("startingLife");
        if(game_settings_json.contains("startingGold"))
            _starting_gold = game_settings_json.at("startingGold");
        if(game_settings_json.contains("fixArmletSkip"))
            _fix_armlet_skip = game_settings_json.at("fixArmletSkip");
        if(game_settings_json.contains("removeTreeCuttingGlitchDrops"))
            _remove_tree_cutting_glitch_drops = game_settings_json.at("removeTreeCuttingGlitchDrops");
        if(game_settings_json.contains("consumableRecordBook"))
            _consumable_record_book = game_settings_json.at("consumableRecordBook");
        if(game_settings_json.contains("consumableSpellBook"))
            _consumable_spell_book = game_settings_json.at("consumableSpellBook");
        if(game_settings_json.contains("removeGumiBoulder"))
            _remove_gumi_boulder = game_settings_json.at("removeGumiBoulder");
        if(game_settings_json.contains("removeTiborRequirement"))
            _remove_tibor_requirement = game_settings_json.at("removeTiborRequirement");
        if(game_settings_json.contains("allTreesVisitedAtStart"))
            _all_trees_visited_at_start = game_settings_json.at("allTreesVisitedAtStart");
        if(game_settings_json.contains("enemiesDamageFactor"))
            _enemies_damage_factor = game_settings_json.at("enemiesDamageFactor");
        if(game_settings_json.contains("enemiesHealthFactor"))
            _enemies_health_factor = game_settings_json.at("enemiesHealthFactor");
        if(game_settings_json.contains("enemiesArmorFactor"))
            _enemies_armor_factor = game_settings_json.at("enemiesArmorFactor");
        if(game_settings_json.contains("enemiesGoldsFactor"))
            _enemies_golds_factor = game_settings_json.at("enemiesGoldsFactor");
        if(game_settings_json.contains("enemiesDropChanceFactor"))    
            _enemies_drop_chance_factor = game_settings_json.at("enemiesDropChanceFactor");
        if(game_settings_json.contains("healthGainedPerLifestock"))
            _health_gained_per_lifestock = game_settings_json.at("healthGainedPerLifestock");
        if(game_settings_json.contains("fastMenuTransitions"))
            _fast_menu_transitions = game_settings_json.at("fastMenuTransitions");

        if(game_settings_json.contains("startingItems"))
        {
            std::map<std::string, uint8_t> starting_items = game_settings_json.at("startingItems");
            for(auto& [item_name, quantity] : starting_items)
            {
                auto it = std::find(_item_names.begin(), _item_names.end(), item_name);
                if(it == _item_names.end())
                    throw LandstalkerException("Unknown item name '" + item_name + "' in starting items section of preset file.");
                uint8_t item_id = std::distance(_item_names.begin(), it);
                _starting_items[item_id] = quantity;
            }
        }
    }

    if(json.contains("randomizerSettings"))
    {
        const Json& randomizer_settings_json = json.at("randomizerSettings");

        if(randomizer_settings_json.contains("allowSpoilerLog"))
            _allow_spoiler_log = randomizer_settings_json.at("allowSpoilerLog");

        if(randomizer_settings_json.contains("spawnLocations"))
            randomizer_settings_json.at("spawnLocations").get_to(_possible_spawn_locations);
        else if(randomizer_settings_json.contains("spawnLocation"))
            _possible_spawn_locations = { randomizer_settings_json.at("spawnLocation") };

        if(randomizer_settings_json.contains("shuffleTrees"))
            _shuffle_tibor_trees = randomizer_settings_json.at("shuffleTrees");
        if(randomizer_settings_json.contains("enemyJumpingInLogic"))
            _enemy_jumping_in_logic = randomizer_settings_json.at("enemyJumpingInLogic");
        if(randomizer_settings_json.contains("damageBoostingInLogic"))
            _damage_boosting_in_logic = randomizer_settings_json.at("damageBoostingInLogic");
        if(randomizer_settings_json.contains("treeCuttingGlitchInLogic"))
            _tree_cutting_glitch_in_logic = randomizer_settings_json.at("treeCuttingGlitchInLogic");
        if(randomizer_settings_json.contains("allowWhistleUsageBehindTrees"))
            _allow_whistle_usage_behind_trees = randomizer_settings_json.at("allowWhistleUsageBehindTrees");

        if(randomizer_settings_json.contains("itemsDistribution"))
        {
            std::map<std::string, uint8_t> items_distribution = randomizer_settings_json.at("itemsDistribution");
            for(auto& [item_name, quantity] : items_distribution)
            {
                auto it = std::find(_item_names.begin(), _item_names.end(), item_name);
                if(it == _item_names.end())
                    throw LandstalkerException("Unknown item name '" + item_name + "' in items distribution section of preset file.");
                uint8_t item_id = std::distance(_item_names.begin(), it);
                _items_distribution[item_id] = quantity;
            }
        }

        if(randomizer_settings_json.contains("hintsDistribution"))
        {
            const Json& hints_distrib_json = randomizer_settings_json.at("hintsDistribution");

            if(hints_distrib_json.contains("regionRequirement"))
                _hints_distribution_region_requirement = hints_distrib_json.at("regionRequirement");
            if(hints_distrib_json.contains("itemRequirement"))
                _hints_distribution_item_requirement = hints_distrib_json.at("itemRequirement");
            if(hints_distrib_json.contains("itemLocation"))
                _hints_distribution_item_location = hints_distrib_json.at("itemLocation");
            if(hints_distrib_json.contains("darkRegion"))
                _hints_distribution_dark_region = hints_distrib_json.at("darkRegion");
            if(hints_distrib_json.contains("joke"))
                _hints_distribution_joke = hints_distrib_json.at("joke");
        }
    }

    if(json.contains("modelPatch"))
    {
        const Json& model_patch_json = json.at("modelPatch");

        if(model_patch_json.contains("items"))
            _model_patch_items = model_patch_json.at("items");
        if(model_patch_json.contains("spawnLocations"))
            _model_patch_spawns = model_patch_json.at("spawnLocations");
        if(model_patch_json.contains("hintSources"))
            _model_patch_hint_sources = model_patch_json.at("hintSources");
    }

    _christmas_event = json.value("christmasEvent", false);

    if(json.contains("world"))
        _world_json = json.at("world");

    if(json.contains("seed"))
        _seed = json.at("seed");
}

void RandomizerOptions::validate() const
{
    if(_jewel_count > 9)
        throw LandstalkerException("Jewel count must be between 0 and 9.");
}

std::vector<std::string> RandomizerOptions::hash_words() const
{
    std::vector<std::string> words = { 
        "EkeEke",   "Nail",        "Horn",       "Fang",      "Magic",      "Ice",       "Thunder",    "Gaia",
        "Mars",     "Moon",        "Saturn",     "Venus",     "Detox",      "Statue",    "Golden",     "Mind",
        "Card",     "Lantern",     "Garlic",     "Paralyze",  "Chicken",    "Death",     "Jypta",      "Sun",
        "Book",     "Lithograph",  "Red",        "Purple",    "Jewel",      "Pawn",      "Gola",       "Nole",
        "Logs",     "Oracle",      "Stone",      "Idol",      "Key",        "Safety",    "Pass",       "Bell",
        "Massan",   "Gumi",        "Ryuma",      "Mercator",  "Verla",      "Destel",    "Kazalt",     "Greedly",
        "Mir",      "Miro",        "Prospero",   "Fara",      "Orc",        "Mushroom",  "Slime",      "Cyclops",
        "Kado",     "Kan",         "Well",       "Dungeon",   "Loria",      "Kayla",     "Wally",      "Ink",
        "Palace",   "Gold",        "Waterfall",  "Shrine",    "Swamp",      "Hideout",   "Greenmaze",  "Mines",
        "Helga",    "Fahl",        "Yard",       "Twinkle",   "Firedemon",  "Spinner",   "Golem",      "Boulder",
        "Kindly",   "Route",       "Shop",       "Green",     "Yellow",     "Blue",      "Fireproof",  "Iron",
        "Spikes",   "Healing",     "Snow",       "Repair",    "Casino",     "Ticket",    "Axe",        "Ribbon",
        "Armlet",   "Einstein",    "Whistle",    "Spell",     "King",       "Dragon",    "Dahl",       "Restoration", 
        "Friday",   "Short",       "Cake",       "Life",      "Stock",      "Zak",       "Duke",       "Dex", 
        "Slasher",  "Marley",      "Nigel",      "Ninja",     "Ghost",      "Tibor",     "Knight",     "Pockets", 
        "Arthur",   "Crypt",       "Mummy",      "Poison",    "Labyrinth",  "Lake",      "Volcano",    "Crate", 
        "Jar",      "Mayor",       "Dexter",     "Treasure",  "Chest"
    };

    std::mt19937 rng(_seed);
    vectools::shuffle(words, rng);
    return { words.begin(), words.begin()+4 };
}

std::string RandomizerOptions::permalink() const
{
    BitstreamWriter bitpack;

    bitpack.pack(std::string(MAJOR_RELEASE));
    
    bitpack.pack(_jewel_count);
    bitpack.pack(_starting_life);
    bitpack.pack(_starting_gold);
    bitpack.pack(_enemies_damage_factor);
    bitpack.pack(_enemies_health_factor);
    bitpack.pack(_enemies_armor_factor);
    bitpack.pack(_enemies_golds_factor);
    bitpack.pack(_enemies_drop_chance_factor);
    bitpack.pack(_health_gained_per_lifestock);
    bitpack.pack(_fast_menu_transitions);

    bitpack.pack(_seed);

    bitpack.pack(_use_armor_upgrades);
    bitpack.pack(_fix_armlet_skip);
    bitpack.pack(_remove_tree_cutting_glitch_drops);
    bitpack.pack(_consumable_record_book);
    bitpack.pack(_consumable_spell_book);
    bitpack.pack(_remove_gumi_boulder);
    bitpack.pack(_remove_tibor_requirement);
    bitpack.pack(_all_trees_visited_at_start);
    bitpack.pack(_allow_spoiler_log);
    bitpack.pack(_shuffle_tibor_trees);
    bitpack.pack(_enemy_jumping_in_logic);
    bitpack.pack(_tree_cutting_glitch_in_logic);
    bitpack.pack(_damage_boosting_in_logic);
    bitpack.pack(_allow_whistle_usage_behind_trees);
    bitpack.pack_array(_items_distribution);
    bitpack.pack(_hints_distribution_region_requirement);
    bitpack.pack(_hints_distribution_item_requirement);
    bitpack.pack(_hints_distribution_item_location);
    bitpack.pack(_hints_distribution_dark_region);
    bitpack.pack(_hints_distribution_joke);

    bitpack.pack_vector(_possible_spawn_locations);

    bitpack.pack_array(_starting_items);
    bitpack.pack_vector(Json::to_msgpack(_model_patch_items));
    bitpack.pack_vector(Json::to_msgpack(_model_patch_spawns));
    bitpack.pack_vector(Json::to_msgpack(_model_patch_hint_sources));
    bitpack.pack_vector(Json::to_msgpack(_world_json));

    return "l" + base64_encode(bitpack.bytes()) + "s";
}

void RandomizerOptions::parse_permalink(const std::string& permalink)
{
    std::vector<uint8_t> bytes = base64_decode(permalink.substr(1, permalink.size() - 2));
    BitstreamReader bitpack(bytes);

    std::string version = bitpack.unpack<std::string>();
    if(version != MAJOR_RELEASE)
        throw WrongVersionException("This permalink comes from an incompatible version of Randstalker (" + version + ").");
    
    _jewel_count = bitpack.unpack<uint8_t>();
    _starting_life = bitpack.unpack<uint8_t>();
    _starting_gold = bitpack.unpack<uint16_t>();
    _enemies_damage_factor = bitpack.unpack<uint16_t>();
    _enemies_health_factor = bitpack.unpack<uint16_t>();
    _enemies_armor_factor = bitpack.unpack<uint16_t>();
    _enemies_golds_factor = bitpack.unpack<uint16_t>();
    _enemies_drop_chance_factor = bitpack.unpack<uint16_t>();
    _health_gained_per_lifestock = bitpack.unpack<uint8_t>();
    _fast_menu_transitions = bitpack.unpack<bool>();

    _seed = bitpack.unpack<uint32_t>();

    _use_armor_upgrades = bitpack.unpack<bool>();
    _fix_armlet_skip = bitpack.unpack<bool>();
    _remove_tree_cutting_glitch_drops = bitpack.unpack<bool>();
    _consumable_record_book = bitpack.unpack<bool>();
    _consumable_spell_book = bitpack.unpack<bool>();
    _remove_gumi_boulder = bitpack.unpack<bool>();
    _remove_tibor_requirement = bitpack.unpack<bool>();
    _all_trees_visited_at_start = bitpack.unpack<bool>();
    _allow_spoiler_log = bitpack.unpack<bool>();
    _shuffle_tibor_trees = bitpack.unpack<bool>();
    _enemy_jumping_in_logic = bitpack.unpack<bool>();
    _tree_cutting_glitch_in_logic = bitpack.unpack<bool>();
    _damage_boosting_in_logic = bitpack.unpack<bool>();
    _allow_whistle_usage_behind_trees = bitpack.unpack<bool>();
    _items_distribution = bitpack.unpack_array<uint8_t, ITEM_COUNT+1>();
    _hints_distribution_region_requirement = bitpack.unpack<uint16_t>();
    _hints_distribution_item_requirement = bitpack.unpack<uint16_t>();
    _hints_distribution_item_location = bitpack.unpack<uint16_t>();
    _hints_distribution_dark_region = bitpack.unpack<uint16_t>();
    _hints_distribution_joke = bitpack.unpack<uint16_t>();

    _possible_spawn_locations = bitpack.unpack_vector<std::string>();

    _starting_items = bitpack.unpack_array<uint8_t, ITEM_COUNT>();
    _model_patch_items = Json::from_msgpack(bitpack.unpack_vector<uint8_t>());
    _model_patch_spawns = Json::from_msgpack(bitpack.unpack_vector<uint8_t>());
    _model_patch_hint_sources = Json::from_msgpack(bitpack.unpack_vector<uint8_t>());
    _world_json = Json::from_msgpack(bitpack.unpack_vector<uint8_t>());
}
