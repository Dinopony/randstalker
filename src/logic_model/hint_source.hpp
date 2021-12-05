#pragma once

#include <vector>
#include <string>
#include "../extlibs/json.hpp"
#include "world_node.hpp"

class HintSource 
{
private:
    std::vector<uint16_t> _text_ids;
    std::vector<std::string*> _string_ptrs;
    std::string _description;
    WorldNode* _node;
    bool _special;
    bool _small_textbox;
    std::string _text;

public:
    HintSource(const std::vector<uint16_t>& text_ids, const std::vector<std::string*>& string_ptrs, 
                const std::string& description, WorldNode* node, bool special, bool small_textbox);

    const std::vector<std::string*>& string_ptrs() const { return _string_ptrs; }
    const std::string& description() const { return _description; }
    WorldNode* node() const { return _node; }
    bool special() const { return _special; }
    bool small_textbox() const { return _small_textbox; }

    const std::string& text() const { return _text; }
    void text(const std::string& text);

    Json to_json() const;
    static HintSource* from_json(const Json& json, const std::map<std::string, WorldNode*>& nodes, 
                                    std::vector<std::string>& game_strings);
};