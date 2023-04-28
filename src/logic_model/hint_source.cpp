#include "hint_source.hpp"

#include <algorithm>
#include <utility>
#include <landstalker-lib/model/world.hpp>
#include <landstalker-lib/tools/game_text.hpp>
#include <landstalker-lib/tools/stringtools.hpp>

HintSource::HintSource(std::vector<uint16_t> text_ids, std::string description,
        WorldNode* node, bool small_textbox,
        std::vector<uint16_t> map_ids, Position position, uint8_t orientation, bool high_palette) :
    _text_ids       (std::move(text_ids)),
    _description    (std::move(description)),
    _node           (node),
    _small_textbox  (small_textbox),
    _map_ids        (std::move(map_ids)),
    _position       (position),
    _orientation    (orientation),
    _high_palette   (high_palette)
{}

std::string HintSource::text() const
{
    std::string pretty_text = _text;

    // Strip the attribute contents from special chars, since it is meant for display
    pretty_text.erase(remove(pretty_text.begin(), pretty_text.end(), '\x1C'), pretty_text.end());
    pretty_text.erase(remove(pretty_text.begin(), pretty_text.end(), '\x1A'), pretty_text.end());
    std::replace(pretty_text.begin(), pretty_text.end(), '\n', ' ');

    return pretty_text;
}

void HintSource::apply_text(World& world)
{
    uint8_t textbox_size = _small_textbox ? 2 : 3;

    std::string initial_game_string;
    if(this->special())
        initial_game_string = GameText(_text, textbox_size).getOutput();
    else
        initial_game_string = GameText(_text, "Foxy", textbox_size).getOutput();

    std::vector<std::string> final_strings;
    if(initial_game_string.size() <= 253)
    {
        final_strings.emplace_back(initial_game_string);
    }
    else
    {
        std::vector<std::string> split_string = stringtools::split(initial_game_string, "\n");
        while(!split_string.empty())
        {
            std::string current_string = split_string[0];
            int i;
            for(i=1 ; i < split_string.size() ; ++i)
            {
                std::string extended_string = current_string + '\n';
                extended_string += split_string[i];
                if(extended_string.size() > 253)
                    break;

                current_string = extended_string;
            }

            split_string.erase(split_string.begin(), split_string.begin() + i);
            final_strings.emplace_back(current_string);
        }
    }

    for(size_t i=0 ; i<final_strings.size() ; ++i)
    {
        if(i >= _text_ids.size())
            _text_ids.emplace_back(world.first_empty_game_string_id(0x4D));

        world.game_strings()[_text_ids[i]] = final_strings[i];
    }
}

Json HintSource::to_json() const
{
    Json json;
    json["description"] = _description;

    if(!_text_ids.empty())
        json["textIds"] = _text_ids;

    if(_node)
        json["nodeId"] = _node->id();
    if(_small_textbox)
        json["smallTextbox"] = _small_textbox;

    if(!_map_ids.empty())
    {
        if(_map_ids.size() == 1)
            json["entity"]["mapId"] = _map_ids[0];
        else if(_map_ids.size() > 1)
            json["entity"]["mapId"] = _map_ids;

        json["entity"]["position"] = _position.to_json();
        json["entity"]["orientation"] = _orientation;
    }

    return json;
}

HintSource* HintSource::from_json(const Json& json, const std::map<std::string, WorldNode*>& nodes)
{
    std::string description = json.at("description");

    std::vector<uint16_t> text_ids;
    if(json.contains("textIds"))
        json.at("textIds").get_to(text_ids);

    bool small_textbox = json.value("smallTextbox", false);

    WorldNode* node = nullptr;
    std::string node_id = json.value("nodeId", "");
    if(!node_id.empty())
        node = nodes.at(node_id);

    std::vector<uint16_t> map_ids;
    Position position;
    uint8_t orientation = 0;
    bool high_palette = false;
    if(json.contains("entity"))
    {
        if(json.at("entity").at("mapId").is_array())
            json.at("entity").at("mapId").get_to(map_ids);
        else
            map_ids = { json.at("entity").at("mapId") };

        position = Position::from_json(json.at("entity").at("position"));

        std::string orientation_str = json.at("entity").at("orientation");
        if(orientation_str == "sw")
            orientation = ENTITY_ORIENTATION_SW;
        else if(orientation_str == "se")
            orientation = ENTITY_ORIENTATION_SE;
        else if(orientation_str == "ne")
            orientation = ENTITY_ORIENTATION_NE;
        else if(orientation_str == "nw")
            orientation = ENTITY_ORIENTATION_NW;
        else
            orientation = std::stoi(orientation_str);

        high_palette = json.at("entity").value("highPalette", false);
    }

    return new HintSource(text_ids, description, node, small_textbox, map_ids, position, orientation, high_palette);
}

