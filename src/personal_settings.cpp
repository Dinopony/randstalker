#include "personal_settings.hpp"
#include <landstalker_lib/exceptions.hpp>
#include <landstalker_lib/tools/argument_dictionary.hpp>
#include <iostream>

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

PersonalSettings::PersonalSettings(const ArgumentDictionary& args)
{
    // Read settings from personal_settings.json file is it's present
    std::ifstream personal_settings_file("./personal_settings.json");
    if(personal_settings_file)
    {
        std::cout << "Reading preset file 'personal_settings.json'...\n";
        Json settings_json;
        personal_settings_file >> settings_json;
        this->parse_json(settings_json);
    }
    else
    {
        std::cout << "Could not open 'personal_settings.json', will use default values.\n";
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
}
