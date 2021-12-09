#pragma once

#include <vector>
#include <string>
#include <landstalker_lib/tools/json.hpp>
#include "world_node.hpp"

class HintSource 
{
private:
    std::vector<uint16_t> _text_ids;
    std::string _description;
    WorldNode* _node = nullptr;
    bool _special;
    bool _small_textbox;
    std::string _text;

public:
    HintSource(std::vector<uint16_t> text_ids, std::string description, WorldNode* node, bool special, bool small_textbox);

    [[nodiscard]] const std::string& description() const { return _description; }
    [[nodiscard]] WorldNode* node() const { return _node; }
    [[nodiscard]] bool special() const { return _special; }
    [[nodiscard]] bool small_textbox() const { return _small_textbox; }

    [[nodiscard]] std::string text() const;
    void text(const std::string& text) { _text = text; }
    void apply_text(World& world);

    [[nodiscard]] Json to_json() const;
    [[nodiscard]] static HintSource* from_json(const Json& json, const std::map<std::string, WorldNode*>& nodes);
};
