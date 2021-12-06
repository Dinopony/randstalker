#include "hint_source.hpp"

#include <algorithm>
#include "../tools/game_text.hpp"

HintSource::HintSource(const std::vector<uint16_t>& text_ids, const std::vector<std::string*>& string_ptrs, 
    const std::string& description, WorldNode* node, bool special, bool small_textbox) :
    _text_ids       (text_ids),
    _string_ptrs    (string_ptrs),
    _description    (description),
    _node         (node),
    _special        (special),
    _small_textbox  (small_textbox),
    _text           ()
{
    // Empty all strings
    for(auto it=_string_ptrs.begin() ; it != _string_ptrs.end() ; ++it)
        *(*it) = " ";
}

void HintSource::text(const std::string& text) 
{ 
    _text = text;

    // Edit the game string contained in World using the string pointer
    std::string& game_string = **_string_ptrs.begin();
    uint8_t textbox_size = _small_textbox ? 2 : 3;
    game_string = GameText(text, textbox_size).getOutput();

    // Strip the attribute contents from special chars, since it is meant for display
    _text.erase(remove(_text.begin(), _text.end(), '\x1C'), _text.end());
    _text.erase(remove(_text.begin(), _text.end(), '\x1A'), _text.end());
    std::replace(_text.begin(), _text.end(), '\n', ' ');
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

HintSource* HintSource::from_json(const Json& json, const std::map<std::string, WorldNode*>& nodes, std::vector<std::string>& game_strings)
{
    std::string description = json.at("description");
    
    std::vector<uint16_t> text_ids = json.at("textIds");
    std::vector<std::string*> string_ptrs;
    for(uint16_t id : text_ids)
        string_ptrs.emplace_back(&(game_strings.at(id)));

    bool special = json.value("special", false);
    bool small_textbox = json.value("smallTextbox", false);

    WorldNode* node = nullptr;
    std::string node_id = json.value("nodeId", "");
    if(!node_id.empty())
        node = nodes.at(node_id);

    return new HintSource(text_ids, string_ptrs, description, node, special, small_textbox);
}