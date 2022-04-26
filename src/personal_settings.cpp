#include "personal_settings.hpp"
#include <landstalker_lib/exceptions.hpp>
#include <landstalker_lib/constants/item_codes.hpp>
#include <landstalker_lib/tools/argument_dictionary.hpp>
#include <iostream>
#include <set>

static Color parse_color_from_name_or_hex(const std::string& string)
{
    std::string lowered = string;
    stringtools::to_lower(lowered);

    if (lowered == "red")              return { "#A22" };
    else if (lowered == "darkred")     return { "#822" };
    else if (lowered == "green")       return { "#6E6" };
    else if (lowered == "darkgreen")   return { "#262" };
    else if (lowered == "blue")        return { "#248" };
    else if (lowered == "brown")       return { "#842" };
    else if (lowered == "darkpurple")  return { "#424" };
    else if (lowered == "darkgray")    return { "#666" };
    else if (lowered == "gray")        return { "#444" };
    else if (lowered == "lightgray")   return { "#AAA" };
    else if (lowered == "salmon")      return { "#E66" };
    else if (lowered == "orange")      return { "#E82" };
    else if (lowered == "white")       return { "#EEE" };

    // Use string as hex code
    return { lowered };
}

PersonalSettings::PersonalSettings(const ArgumentDictionary& args, const std::array<std::string, ITEM_COUNT+1>& item_names)
{
    _item_names = item_names;

    _inventory_order = {
            ITEM_RECORD_BOOK,   ITEM_SPELL_BOOK,
            ITEM_EKEEKE,        ITEM_DAHL,
            ITEM_GOLDEN_STATUE, ITEM_GAIA_STATUE,
            ITEM_RESTORATION,   ITEM_DETOX_GRASS,
            ITEM_MIND_REPAIR,   ITEM_ANTI_PARALYZE,
            ITEM_KEY,           ITEM_IDOL_STONE,
            ITEM_GARLIC,        ITEM_LOGS,
            ITEM_GOLA_EYE,      ITEM_GOLA_NAIL,
            ITEM_GOLA_HORN,     ITEM_GOLA_FANG,
            ITEM_DEATH_STATUE,  ITEM_CASINO_TICKET,
            ITEM_SHORT_CAKE,    ITEM_PAWN_TICKET,
            ITEM_ORACLE_STONE,  ITEM_LITHOGRAPH,
            ITEM_RED_JEWEL,     ITEM_PURPLE_JEWEL,
            ITEM_GREEN_JEWEL,   ITEM_BLUE_JEWEL,
            ITEM_YELLOW_JEWEL,  ITEM_SAFETY_PASS,
            ITEM_AXE_MAGIC,     ITEM_ARMLET,
            ITEM_SUN_STONE,     ITEM_BUYER_CARD,
            ITEM_LANTERN,       ITEM_EINSTEIN_WHISTLE,
            ITEM_STATUE_JYPTA,  ITEM_BELL,
            ITEM_BLUE_RIBBON,   0xFF
    };

    // Read settings from personal_settings.json file is it's present
    std::string settings_path = args.get_string("personalsettings");
    if(settings_path.empty())
        settings_path = "./personal_settings.json";

    std::ifstream personal_settings_file(settings_path);
    if(personal_settings_file)
    {
        std::cout << "Reading personal settings file '" << settings_path << "'...\n";
        Json settings_json;
        personal_settings_file >> settings_json;
        this->parse_json(settings_json);
    }
    else
    {
        std::cout << "Could not open '"<< settings_path << "', will use default values.\n";
    }

    // Parse arguments from command line
    if(args.contains("ingametracker"))
        _add_ingame_item_tracker = args.get_boolean("ingametracker");

    if(args.contains("hudcolor"))
    {
        std::string hud_color_as_string = args.get_string("hudcolor");
        _hud_color = parse_color_from_name_or_hex(hud_color_as_string);
    }

    if(args.contains("nigelcolor"))
    {
        std::string nigel_color_as_string = args.get_string("nigelcolor");
        _nigel_colors.first = parse_color_from_name_or_hex(nigel_color_as_string);
        _nigel_colors.second = _nigel_colors.first.subtract(0x40);
    }
}

void PersonalSettings::parse_json(const Json& json)
{
    if(json.contains("inGameTracker"))
        _add_ingame_item_tracker = json.at("inGameTracker");

    if(json.contains("hudColor"))
        _hud_color = Color::from_json(json.at("hudColor"));

    if(json.contains("nigelColor"))
    {
        if(json.at("nigelColor").is_array())
        {
            _nigel_colors.first = Color::from_json(json.at("nigelColor")[0]);
            _nigel_colors.second = Color::from_json(json.at("nigelColor")[1]);
        }
        else
        {
            _nigel_colors.first = Color::from_json(json.at("nigelColor"));
            _nigel_colors.second = _nigel_colors.first.subtract(0x40);
        }
    }

    if(json.contains("removeMusic"))
        _remove_music = json.at("removeMusic");

    if(json.contains("inventoryOrder"))
    {
        _inventory_order.fill(0xFF);

        std::set<uint8_t> mandatory_items = {
            ITEM_KEY, ITEM_IDOL_STONE, ITEM_GARLIC, ITEM_LOGS, ITEM_GOLA_EYE, ITEM_EINSTEIN_WHISTLE
        };

        std::vector<std::string> items = json.at("inventoryOrder");
        for(uint8_t i=0 ; i < items.size() && i < _inventory_order.size() ; ++i)
        {
            auto it = std::find(_item_names.begin(), _item_names.end(), items[i]);
            if(it == _item_names.end())
                throw LandstalkerException("Unknown item name '" + items[i] + "' in inventory order section of personal settings file.");
            uint8_t item_id = std::distance(_item_names.begin(), it);
            _inventory_order[i] = item_id;
            if(mandatory_items.contains(item_id))
                mandatory_items.erase(item_id);
        }

        if(!mandatory_items.empty())
            throw LandstalkerException("Cannot omit " + _item_names[*mandatory_items.begin()] + " from inventory order since it could make seeds uncompletable");
    }
}
