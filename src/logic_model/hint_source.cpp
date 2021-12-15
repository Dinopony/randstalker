#include "hint_source.hpp"

#include <algorithm>
#include <utility>
#include <landstalker_lib/model/world.hpp>
#include <landstalker_lib/tools/game_text.hpp>

HintSource::HintSource(std::vector<uint16_t> text_ids, std::string description,
        WorldNode* node, bool small_textbox,
        std::vector<uint16_t> map_ids, Position position, uint8_t orientation) :
    _text_ids       (std::move(text_ids)),
    _description    (std::move(description)),
    _node           (node),
    _small_textbox  (small_textbox),
    _map_ids        (std::move(map_ids)),
    _position       (position),
    _orientation    (orientation)
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
    // Edit the game string contained in World referenced by the text id
    if(!_text.empty())
    {
        for(uint16_t text_id : _text_ids)
            world.game_strings()[text_id].clear();

        if(text_ids().empty())
        {
            // Associate an empty game_string ID with this hint source (take it from 0x4D since strings below 0x4D
            // cannot be referenced by dialogue script commands)
            _text_ids = { world.first_empty_game_string_id(0x4D) };
        }

        std::string& game_string = world.game_strings().at(_text_ids[0]);
        uint8_t textbox_size = _small_textbox ? 2 : 3;

        if(this->has_entity())
            game_string = GameText(_text, "Foxy", textbox_size).getOutput();
        else
            game_string = GameText(_text, textbox_size).getOutput();
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
    }

    return new HintSource(text_ids, description, node, small_textbox, map_ids, position, orientation);
}

