#include "hint_source.hpp"

#include <algorithm>
#include <utility>
#include <landstalker_lib/model/world.hpp>
#include <landstalker_lib/tools/game_text.hpp>

HintSource::HintSource(std::vector<uint16_t> text_ids, std::string description, WorldNode* node, bool special, bool small_textbox) :
    _text_ids       (std::move(text_ids)),
    _description    (std::move(description)),
    _node           (node),
    _special        (special),
    _small_textbox  (small_textbox)
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

        std::string& game_string = world.game_strings().at(_text_ids[0]);
        uint8_t textbox_size = _small_textbox ? 2 : 3;
        game_string = GameText(_text, textbox_size).getOutput();
    }
}

Json HintSource::to_json() const
{
    Json json;
    json["description"] = _description;
    json["textIds"] = _text_ids;

    if(_special)
        json["special"] = true;
    else
        json["nodeId"] = _node->id();
    
    if(_small_textbox)
        json["smallTextbox"] = _small_textbox;

    return json;
}

HintSource* HintSource::from_json(const Json& json, const std::map<std::string, WorldNode*>& nodes)
{
    std::string description = json.at("description");
    
    std::vector<uint16_t> text_ids = json.at("textIds");

    bool special = json.value("special", false);
    bool small_textbox = json.value("smallTextbox", false);

    WorldNode* node = nullptr;
    std::string node_id = json.value("nodeId", "");
    if(!node_id.empty())
        node = nodes.at(node_id);

    return new HintSource(text_ids, description, node, special, small_textbox);
}
