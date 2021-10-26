#include "randomizer_options.hpp"

#include <iostream>
#include "extlibs/base64.hpp"
#include "tools/tools.hpp"
#include "exceptions.hpp"

RandomizerOptions::RandomizerOptions() :
    _jewel_count              (2),
    _use_armor_upgrades       (true),
    _dungeonSignHints         (false),
    _startingLife             (0),
    _startingGold             (0),
    _starting_items           ({{"Record Book",1}}),
    _itemPrices               (),
    _fix_armlet_skip          (true),
    _fix_tree_cutting_glitch  (true),
    _item_max_quantities      (),
    _consumable_record_book   (false),

    _seed                     (0),
    _allow_spoiler_log        (true),
    _fillingRate              (DEFAULT_FILLING_RATE),
    _shuffle_tibor_trees      (false), 
    _ghost_jumping_in_logic   (false),
    _mandatory_items          (nullptr),
    _filler_items             (nullptr),

    _inputRomPath             ("./input.md"),
    _output_rom_path          ("./"),
    _spoiler_log_path         (""),
    _debugLogPath             (""),
    _pauseAfterGeneration     (true),
    _add_ingame_item_tracker  (false),
    _hud_color                ("default"),

    _plando_enabled           (false),
    _plando_json              ()
{}

RandomizerOptions::RandomizerOptions(const ArgumentDictionary& args) : RandomizerOptions()
{
    std::string plando_path = args.getString("plando");
    if(!plando_path.empty())
    {
        _plando_enabled = true;

        std::cout << "Reading plando file '" << plando_path << "'...\n\n";
        std::ifstream plando_file(plando_path);
        if(!plando_file)
            throw RandomizerException("Could not open plando file at given path '" + plando_path + "'");

        plando_file >> _plando_json;
        if(_plando_json.contains("plando_permalink"))
            _plando_json = Json::from_msgpack(base64_decode(_plando_json.at("plando_permalink")));

        this->parse_json(_plando_json);
    }

    std::string permalink_string = args.getString("permalink");
    if(!permalink_string.empty() && !_plando_enabled) 
    {
        this->parse_permalink(permalink_string);
    }
    else
    {
        // Parse seed from args, or generate a random one if it's missing
        std::string seed_string = args.getString("seed", "random");
        try {
            _seed = (uint32_t) std::stoul(seed_string);
        } catch (std::invalid_argument&) {
            _seed = (uint32_t) std::chrono::system_clock::now().time_since_epoch().count();
        }

        std::string preset_path = args.getString("preset");
        if(!preset_path.empty() && !_plando_enabled)
        {
            std::ifstream preset_file(preset_path);
            if(!preset_file)
                throw RandomizerException("Could not open preset file at given path '" + preset_path + "'");

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
    if(_mandatory_items)
        delete _mandatory_items;
    if(_filler_items)
        delete _filler_items;
}

void RandomizerOptions::parse_arguments(const ArgumentDictionary& args)
{
    if(args.contains("spawnlocation"))
    {
        std::string spawn_name = args.getString("spawnlocation");
        Tools::toLower(spawn_name);
        if(spawn_name == "random")
            _possible_spawn_locations = {};
        else
            _possible_spawn_locations = { spawn_name };
    }

    if(args.contains("jewelcount"))           _jewel_count = args.getInteger("jewelcount");
    if(args.contains("armorupgrades"))        _use_armor_upgrades = args.getBoolean("armorupgrades");
    if(args.contains("norecordbook"))         _starting_items["Record Book"] = 0;
    if(args.contains("dungeonsignhints"))     _dungeonSignHints = args.getBoolean("dungeonsignhints");
    if(args.contains("startinglife"))         _startingLife = args.getInteger("startinglife");

    if(args.contains("fillingrate"))          _fillingRate = args.getDouble("fillingrate");
    if(args.contains("shuffletrees"))         _shuffle_tibor_trees = args.getBoolean("shuffletrees");
    if(args.contains("allowspoilerlog"))      _allow_spoiler_log = args.getBoolean("allowspoilerlog");
}

void RandomizerOptions::parse_personal_settings(const ArgumentDictionary& args)
{
    // Personal options (not included in permalink)
    if(args.contains("inputrom"))        _inputRomPath = args.getString("inputrom");
    if(args.contains("outputrom"))       _output_rom_path = args.getString("outputrom");
    if(args.contains("outputlog"))       _spoiler_log_path = args.getString("outputlog");
    if(args.contains("debuglog"))        _debugLogPath = args.getString("debuglog");
    if(args.contains("pause"))           _pauseAfterGeneration = args.getBoolean("pause");
    if(args.contains("ingametracker"))   _add_ingame_item_tracker = args.getBoolean("ingametracker");    
    if(args.contains("hudcolor"))        _hud_color = args.getString("hudcolor");
}

Json RandomizerOptions::to_json() const
{
    Json json;

    // Game settings 
    json["gameSettings"]["jewelCount"] = _jewel_count;
    json["gameSettings"]["armorUpgrades"] = _use_armor_upgrades;
    json["gameSettings"]["dungeonSignHints"] = _dungeonSignHints;
    json["gameSettings"]["startingGold"] = _startingGold;
    json["gameSettings"]["startingItems"] = _starting_items;
    json["gameSettings"]["itemPrices"] = _itemPrices;
    json["gameSettings"]["fixArmletSkip"] = _fix_armlet_skip;
    json["gameSettings"]["fixTreeCuttingGlitch"] = _fix_tree_cutting_glitch;
    json["gameSettings"]["itemMaxQuantities"] = _item_max_quantities;
    json["gameSettings"]["consumableRecordBook"] = _consumable_record_book;
    if(_startingLife > 0)
        json["gameSettings"]["startingLife"] = _startingLife;

    // Randomizer settings
    json["randomizerSettings"]["allowSpoilerLog"] = _allow_spoiler_log;
    json["randomizerSettings"]["fillingRate"] = _fillingRate;
    json["randomizerSettings"]["spawnLocations"] = _possible_spawn_locations;
    json["randomizerSettings"]["shuffleTrees"] = _shuffle_tibor_trees;
    json["randomizerSettings"]["ghostJumpingInLogic"] = _ghost_jumping_in_logic;

    if(_mandatory_items)
        json["randomizerSettings"]["mandatoryItems"] = *_mandatory_items;
    if(_filler_items)
        json["randomizerSettings"]["fillerItems"] = *_filler_items;

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
        if(game_settings_json.contains("dungeonSignHints"))
            _dungeonSignHints = game_settings_json.at("dungeonSignHints");
        if(game_settings_json.contains("startingLife"))
            _startingLife = game_settings_json.at("startingLife");
        if(game_settings_json.contains("startingGold"))
            _startingGold = game_settings_json.at("startingGold");
        if(game_settings_json.contains("startingItems"))
        {
            std::map<std::string, uint8_t> startingItems = game_settings_json.at("startingItems");
            for(auto& [itemName, quantity] : startingItems)
                _starting_items[itemName] = quantity;
        }
        if(game_settings_json.contains("itemPrices"))
            _itemPrices = game_settings_json.at("itemPrices");
        if(game_settings_json.contains("fixArmletSkip"))
            _fix_armlet_skip = game_settings_json.at("fixArmletSkip");
        if(game_settings_json.contains("fixTreeCuttingGlitch"))
            _fix_tree_cutting_glitch = game_settings_json.at("fixTreeCuttingGlitch");
        if(game_settings_json.contains("itemMaxQuantities"))
            _item_max_quantities = game_settings_json.at("itemMaxQuantities");
        if(game_settings_json.contains("consumableRecordBook"))
            _consumable_record_book = game_settings_json.at("consumableRecordBook");
    }

    if(json.contains("randomizerSettings") && !_plando_enabled)
    {
        const Json& randomizer_settings_json = json.at("randomizerSettings");

        if(randomizer_settings_json.contains("allowSpoilerLog"))
            _allow_spoiler_log = randomizer_settings_json.at("allowSpoilerLog");
        if(randomizer_settings_json.contains("fillingRate"))
            _fillingRate = randomizer_settings_json.at("fillingRate");

        if(randomizer_settings_json.contains("spawnLocations"))
            randomizer_settings_json.at("spawnLocations").get_to(_possible_spawn_locations);
        else if(randomizer_settings_json.contains("spawnLocation"))
            _possible_spawn_locations = { randomizer_settings_json.at("spawnLocation") };

        if(randomizer_settings_json.contains("shuffleTrees"))
            _shuffle_tibor_trees = randomizer_settings_json.at("shuffleTrees");
        if(randomizer_settings_json.contains("ghostJumpingInLogic"))
            _ghost_jumping_in_logic = randomizer_settings_json.at("ghostJumpingInLogic");

        if(randomizer_settings_json.contains("mandatoryItems"))
        {
            _mandatory_items = new std::map<std::string, uint16_t>();
            *(_mandatory_items) = randomizer_settings_json.at("mandatoryItems");
        }

        if(randomizer_settings_json.contains("fillerItems"))
        {
            _filler_items = new std::map<std::string, uint16_t>();
            *(_filler_items) = randomizer_settings_json.at("fillerItems");
        }
    }

    if(json.contains("seed") && !_plando_enabled)
        _seed = json.at("seed");
}

Json RandomizerOptions::personal_settings_as_json() const
{
    Json json;

    json["inputROMPath"] = _inputRomPath;
    json["outputROMPath"] = _output_rom_path;
    json["spoilerLogPath"] = _spoiler_log_path;
    json["addIngameItemTracker"] = _add_ingame_item_tracker;
    json["hudColor"] = _hud_color;

    return json;
}

void RandomizerOptions::validate()
{
    if(_jewel_count > 9)
        throw RandomizerException("Jewel count must be between 0 and 9.");

    // Clean output ROM path and determine if it's a directory or a file
    bool output_path_is_file = Tools::endsWith(_output_rom_path, ".md") || Tools::endsWith(_output_rom_path, ".bin");
    if(!output_path_is_file && *_output_rom_path.rbegin() != '/')
        _output_rom_path += "/";

    // Clean output log path and if it wasn't specified, give it an appropriate default value
    if(_spoiler_log_path.empty())
    {
        if(output_path_is_file)
            _spoiler_log_path = "./"; // outputRomPath points to a file, use cwd for the spoiler log
        else
            _spoiler_log_path = _output_rom_path; // outputRomPath points to a directory, use the same for the spoiler log
    }
    if(!Tools::endsWith(_spoiler_log_path, ".json") && *_spoiler_log_path.rbegin() != '/')
        _spoiler_log_path += "/";

    // Add the filename afterwards
    if(*_output_rom_path.rbegin() == '/')
        _output_rom_path += this->hash_sentence() + ".md";
    if(*_spoiler_log_path.rbegin() == '/')
        _spoiler_log_path += this->hash_sentence() + ".json";
}

std::vector<std::string> RandomizerOptions::hash_words() const
{
    std::vector<std::string> words = {
        "EkeEke", "Nail", "Horn", "Fang", "Magic", "Ice", "Thunder", "Gaia", "Fireproof", "Iron", "Spikes", "Healing", "Snow",
        "Mars", "Moon", "Saturn", "Venus", "Detox", "Statue", "Golden", "Mind", "Repair", "Casino", "Ticket", "Axe", "Ribbon",
        "Card", "Lantern", "Garlic", "Paralyze", "Chicken", "Death", "Jypta", "Sun", "Armlet", "Einstein", "Whistle", "Spell",
        "Book", "Lithograph", "Red", "Purple", "Jewel", "Pawn", "Ticket", "Gola", "Nole", "King", "Dragon", "Dahl", "Restoration",
        "Logs", "Oracle", "Stone", "Idol", "Key", "Safety", "Pass", "Bell", "Short", "Cake", "Life", "Stock", "Zak", "Duke",
        "Massan", "Gumi", "Ryuma", "Mercator", "Verla", "Destel", "Kazalt", "Greedly", "Dex", "Slasher", "Marley", "Nigel", "Friday",
        "Mir", "Miro", "Prospero", "Fara", "Orc", "Mushroom", "Slime", "Cyclops", "Ninja", "Ghost", "Tibor", "Knight", "Pockets",
        "Kado", "Kan", "Well", "Dungeon", "Loria", "Kayla", "Wally", "Ink", "Arthur", "Crypt", "Mummy", "Poison", "Labyrinth",
        "Palace", "Gold", "Waterfall", "Shrine", "Swamp", "Hideout", "Greenmaze", "Mines", "Lake", "Volcano", "Crate", "Jar", 
        "Helga", "Fahl", "Yard", "Twinkle", "Firedemon", "Spinner", "Golem", "Boulder"
    };

    std::mt19937 rng(_seed);
    Tools::shuffle(words, rng);
    return std::vector<std::string>(words.begin(), words.begin() + 4);
}

std::string RandomizerOptions::permalink() const
{
    RandomizerOptions default_options;
    Json defaultJson = default_options.to_json();
    
    Json json_patch = Json::diff(defaultJson, this->to_json());
    Json permalink_json;

    // Apply patch on an empty JSON
    for (Json& patch_piece : json_patch)
    {
        Json* current_json = &permalink_json;

        const std::string& path = patch_piece["path"];
        std::vector<std::string> path_parts = Tools::split(path, "/");
        for (size_t i=1 ; i < path_parts.size(); ++i)
        {
            if (i < path_parts.size() - 1)
            {
                if(!current_json->contains(path_parts[i]))
                    (*current_json)[path_parts[i]] = Json();
                current_json = &(current_json->at(path_parts[i]));
            }
            else if(path_parts[i] == "-")
                (*current_json).push_back(patch_piece["value"]);
            else
                (*current_json)[path_parts[i]] = patch_piece["value"];
        }
    }

    permalink_json["version"] = MAJOR_RELEASE;
    permalink_json["seed"] = _seed;

    return "L" + base64_encode(Json::to_msgpack(permalink_json)) + "S";
}

void RandomizerOptions::parse_permalink(const std::string& permalink)
{
    try {
        Json permalink_json = Json::from_msgpack(base64_decode(permalink.substr(1, permalink.size()-2)));

        std::string version = permalink_json.at("version");
        if(version != MAJOR_RELEASE) {
            throw WrongVersionException("This permalink comes from an incompatible version of Randstalker (" + version + ").");
        }

        this->parse_json(permalink_json);
    }
    catch(std::exception&) {
        throw RandomizerException("Invalid permalink given.");
    }
}
