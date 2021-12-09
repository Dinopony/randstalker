#include "randomizer_options.hpp"

#include <iostream>

#include <landstalker_lib/tools/tools.hpp>
#include <landstalker_lib/exceptions.hpp>

#include "tools/bitpack.hpp"
#include "tools/base64.hpp"

RandomizerOptions::RandomizerOptions(const ArgumentDictionary& args) : RandomizerOptions()
{
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

    this->parse_personal_settings(args);
    this->validate();
}

RandomizerOptions::~RandomizerOptions()
{
    delete _mandatory_items;
    delete _filler_items;
}

void RandomizerOptions::parse_arguments(const ArgumentDictionary& args)
{
    if(args.contains("spawnlocation"))
    {
        std::string spawn_name = args.get_string("spawnlocation");
        tools::to_lower(spawn_name);
        if(spawn_name == "random")
            _possible_spawn_locations = {};
        else
            _possible_spawn_locations = { spawn_name };
    }

    if(args.contains("jewelcount"))           _jewel_count = args.get_integer("jewelcount");
    if(args.contains("armorupgrades"))        _use_armor_upgrades = args.get_boolean("armorupgrades");
    if(args.contains("norecordbook"))         _starting_items["Record Book"] = 0;
    if(args.contains("nospellbook"))          _starting_items["Spell Book"] = 0;
    if(args.contains("startinglife"))         _startingLife = args.get_integer("startinglife");

    if(args.contains("itemsourceswindow"))    _item_sources_window = args.get_integer("itemsourceswindow");
    if(args.contains("shuffletrees"))         _shuffle_tibor_trees = args.get_boolean("shuffletrees");
    if(args.contains("allowspoilerlog"))      _allow_spoiler_log = args.get_boolean("allowspoilerlog");
}

void RandomizerOptions::parse_personal_settings(const ArgumentDictionary& args)
{
    // Personal options (not included in permalink)
    if(args.contains("ingametracker"))   _add_ingame_item_tracker = args.get_boolean("ingametracker");    
    if(args.contains("hudcolor"))
    {
        std::string hud_color_as_string = args.get_string("hudcolor");
        
        try {
            _hud_color = (uint16_t)std::stoul(hud_color_as_string);
        } 
        catch(std::invalid_argument&)
        {
            tools::to_lower(hud_color_as_string);

            if (hud_color_as_string == "red")              _hud_color = 0x228;
            else if (hud_color_as_string == "darkred")     _hud_color = 0x226;
            else if (hud_color_as_string == "green")       _hud_color = 0x262;
            else if (hud_color_as_string == "blue")        _hud_color = 0x842;
            else if (hud_color_as_string == "brown")       _hud_color = 0x248;
            else if (hud_color_as_string == "darkpurple")  _hud_color = 0x424;
            else if (hud_color_as_string == "darkgray")    _hud_color = 0x222;
            else if (hud_color_as_string == "gray")        _hud_color = 0x444;
            else if (hud_color_as_string == "lightgray")   _hud_color = 0x666;
        }
    }        
}

Json RandomizerOptions::to_json() const
{
    Json json;

    // Game settings 
    json["gameSettings"]["jewelCount"] = _jewel_count;
    json["gameSettings"]["armorUpgrades"] = _use_armor_upgrades;
    json["gameSettings"]["startingGold"] = _startingGold;
    json["gameSettings"]["startingItems"] = _starting_items;
    json["gameSettings"]["fixArmletSkip"] = _fix_armlet_skip;
    json["gameSettings"]["fixTreeCuttingGlitch"] = _fix_tree_cutting_glitch;
    json["gameSettings"]["consumableRecordBook"] = _consumable_record_book;
    json["gameSettings"]["removeGumiBoulder"] = _remove_gumi_boulder;
    json["gameSettings"]["removeTiborRequirement"] = _remove_tibor_requirement;
    json["gameSettings"]["allTreesVisitedAtStart"] = _all_trees_visited_at_start;
    json["gameSettings"]["enemiesDamageFactor"] = _enemies_damage_factor;
    json["gameSettings"]["enemiesHealthFactor"] = _enemies_health_factor;
    json["gameSettings"]["enemiesArmorFactor"] = _enemies_armor_factor;
    json["gameSettings"]["enemiesGoldsFactor"] = _enemies_golds_factor;
    json["gameSettings"]["enemiesDropChanceFactor"] = _enemies_drop_chance_factor;
    if(_startingLife > 0)
        json["gameSettings"]["startingLife"] = _startingLife;

    // Randomizer settings
    json["randomizerSettings"]["allowSpoilerLog"] = _allow_spoiler_log;
    json["randomizerSettings"]["itemSourcesWindow"] = _item_sources_window;
    json["randomizerSettings"]["spawnLocations"] = _possible_spawn_locations;
    json["randomizerSettings"]["shuffleTrees"] = _shuffle_tibor_trees;
    json["randomizerSettings"]["ghostJumpingInLogic"] = _ghost_jumping_in_logic;
    json["randomizerSettings"]["damageBoostingInLogic"] = _damage_boosting_in_logic;

    if(_mandatory_items)
        json["randomizerSettings"]["mandatoryItems"] = *_mandatory_items;
    if(_filler_items)
        json["randomizerSettings"]["fillerItems"] = *_filler_items;

    if(!_model_patch_items.empty())
        json["modelPatch"]["items"] = _model_patch_items;
    if(!_model_patch_spawns.empty())
        json["modelPatch"]["spawnLocations"] = _model_patch_spawns;

    return json;
}

void RandomizerOptions::parse_json(const Json& json)
{
    if(json.contains("gameSettings"))
    {
        const Json& game_settings_json = json.at("gameSettings");

        if(game_settings_json.contains("jewelCount"))            
            _jewel_count = game_settings_json.at("jewelCount");
        if(game_settings_json.contains("armorUpgrades"))
            _use_armor_upgrades = game_settings_json.at("armorUpgrades");
        if(game_settings_json.contains("startingLife"))
            _startingLife = game_settings_json.at("startingLife");
        if(game_settings_json.contains("startingGold"))
            _startingGold = game_settings_json.at("startingGold");
        if(game_settings_json.contains("fixArmletSkip"))
            _fix_armlet_skip = game_settings_json.at("fixArmletSkip");
        if(game_settings_json.contains("fixTreeCuttingGlitch"))
            _fix_tree_cutting_glitch = game_settings_json.at("fixTreeCuttingGlitch");
        if(game_settings_json.contains("consumableRecordBook"))
            _consumable_record_book = game_settings_json.at("consumableRecordBook");
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

        if(game_settings_json.contains("startingItems"))
        {
            std::map<std::string, uint8_t> startingItems = game_settings_json.at("startingItems");
            for(auto& [itemName, quantity] : startingItems)
                _starting_items[itemName] = quantity;
        }
    }

    if(json.contains("randomizerSettings"))
    {
        const Json& randomizer_settings_json = json.at("randomizerSettings");

        if(randomizer_settings_json.contains("allowSpoilerLog"))
            _allow_spoiler_log = randomizer_settings_json.at("allowSpoilerLog");
        if(randomizer_settings_json.contains("itemSourcesWindow"))
            _item_sources_window = randomizer_settings_json.at("itemSourcesWindow");

        if(randomizer_settings_json.contains("spawnLocations"))
            randomizer_settings_json.at("spawnLocations").get_to(_possible_spawn_locations);
        else if(randomizer_settings_json.contains("spawnLocation"))
            _possible_spawn_locations = { randomizer_settings_json.at("spawnLocation") };

        if(randomizer_settings_json.contains("shuffleTrees"))
            _shuffle_tibor_trees = randomizer_settings_json.at("shuffleTrees");
        if(randomizer_settings_json.contains("ghostJumpingInLogic"))
            _ghost_jumping_in_logic = randomizer_settings_json.at("ghostJumpingInLogic");
        if(randomizer_settings_json.contains("damageBoostingInLogic"))
            _damage_boosting_in_logic = randomizer_settings_json.at("damageBoostingInLogic");

        if(randomizer_settings_json.contains("mandatoryItems"))
        {
            try {            
                _mandatory_items = new std::map<std::string, uint16_t>();
                *(_mandatory_items) = randomizer_settings_json.at("mandatoryItems");
            } 
            catch(Json::exception&) {
                throw LandstalkerException("Error while parsing mandatory items from preset");
            }
        }

        if(randomizer_settings_json.contains("fillerItems"))
        {
            try {
                _filler_items = new std::map<std::string, uint16_t>();
                *(_filler_items) = randomizer_settings_json.at("fillerItems");
            } catch (Json::exception&) {
                throw LandstalkerException("Error while parsing filler items from preset");
            }
        }
    }

    if(json.contains("modelPatch"))
    {
        const Json& model_patch_json = json.at("modelPatch");

        if(model_patch_json.contains("items"))
            _model_patch_items = model_patch_json.at("items");
        if(model_patch_json.contains("spawnLocations"))
            _model_patch_spawns = model_patch_json.at("spawnLocations");
    }

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
    tools::shuffle(words, rng);
    return { words.begin(), words.begin()+4 };
}

std::string RandomizerOptions::permalink() const
{
    Bitpack bitpack;

    bitpack.pack(std::string(MAJOR_RELEASE));
    
    bitpack.pack(_jewel_count);
    bitpack.pack(_startingLife);
    bitpack.pack(_startingGold);
    bitpack.pack(_enemies_damage_factor);
    bitpack.pack(_enemies_health_factor);
    bitpack.pack(_enemies_armor_factor);
    bitpack.pack(_enemies_golds_factor);
    bitpack.pack(_enemies_drop_chance_factor);

    bitpack.pack(_seed);
    bitpack.pack(_item_sources_window);

    bitpack.pack(_use_armor_upgrades);
    bitpack.pack(_fix_armlet_skip);
    bitpack.pack(_fix_tree_cutting_glitch);
    bitpack.pack(_consumable_record_book);
    bitpack.pack(_remove_gumi_boulder);
    bitpack.pack(_remove_tibor_requirement);
    bitpack.pack(_all_trees_visited_at_start);
    bitpack.pack(_allow_spoiler_log);
    bitpack.pack(_shuffle_tibor_trees);
    bitpack.pack(_ghost_jumping_in_logic);
    bitpack.pack(_damage_boosting_in_logic);

    bitpack.pack_vector(_possible_spawn_locations);
    bitpack.pack(_mandatory_items ? Json(*_mandatory_items) : Json());
    bitpack.pack(_filler_items ? Json(*_filler_items) : Json());
    bitpack.pack_map(_starting_items);
    bitpack.pack(_model_patch_items);
    bitpack.pack(_model_patch_spawns);
    bitpack.pack(_world_json);

    return "l" + base64_encode(bitpack.to_bytes()) + "s";
}

void RandomizerOptions::parse_permalink(const std::string& permalink)
{
    std::vector<uint8_t> bytes = base64_decode(permalink.substr(1, permalink.size() - 2));
    Bitpack bitpack(bytes);

    std::string version = bitpack.unpack<std::string>();
    if(version != MAJOR_RELEASE)
        throw WrongVersionException("This permalink comes from an incompatible version of Randstalker (" + version + ").");
    
    _jewel_count = bitpack.unpack<uint8_t>();
    _startingLife = bitpack.unpack<uint8_t>();
    _startingGold = bitpack.unpack<uint16_t>();
    _enemies_damage_factor = bitpack.unpack<uint16_t>();
    _enemies_health_factor = bitpack.unpack<uint16_t>();
    _enemies_armor_factor = bitpack.unpack<uint16_t>();
    _enemies_golds_factor = bitpack.unpack<uint16_t>();
    _enemies_drop_chance_factor = bitpack.unpack<uint16_t>();

    _seed = bitpack.unpack<uint32_t>();
    _item_sources_window = bitpack.unpack<uint16_t>();

    _use_armor_upgrades = bitpack.unpack<bool>();
    _fix_armlet_skip = bitpack.unpack<bool>();
    _fix_tree_cutting_glitch = bitpack.unpack<bool>();
    _consumable_record_book = bitpack.unpack<bool>();
    _remove_gumi_boulder = bitpack.unpack<bool>();
    _remove_tibor_requirement = bitpack.unpack<bool>();
    _all_trees_visited_at_start = bitpack.unpack<bool>();
    _allow_spoiler_log = bitpack.unpack<bool>();
    _shuffle_tibor_trees = bitpack.unpack<bool>();
    _ghost_jumping_in_logic = bitpack.unpack<bool>();
    _damage_boosting_in_logic = bitpack.unpack<bool>();

    _possible_spawn_locations = bitpack.unpack_vector<std::string>();

    Json mandatory_items_json = bitpack.unpack<Json>();
    if(!mandatory_items_json.is_null())
    {
        _mandatory_items = new std::map<std::string, uint16_t>();
        mandatory_items_json.get_to(*_mandatory_items);
    }
    else _mandatory_items = nullptr;

    Json filler_items_json = bitpack.unpack<Json>();
    if(!filler_items_json.is_null())
    {
        _filler_items = new std::map<std::string, uint16_t>();
        filler_items_json.get_to(*_filler_items); 
    }
    else _filler_items = nullptr;

    _starting_items = bitpack.unpack_map<std::string, uint8_t>();
    _model_patch_items = bitpack.unpack<Json>();
    _model_patch_spawns = bitpack.unpack<Json>();
    _world_json = bitpack.unpack<Json>();
}
