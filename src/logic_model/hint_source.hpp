#pragma once

#include <utility>
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
    bool _small_textbox = false;
    std::string _text;

    std::vector<uint16_t> _map_ids;
    Position _position;
    uint8_t _orientation = 0;
    bool _high_palette = false;

public:
    HintSource(std::vector<uint16_t> text_ids, std::string description,
               WorldNode* node, bool small_textbox,
               std::vector<uint16_t> map_ids, Position position, uint8_t orientation, bool high_palette);

    [[nodiscard]] const std::string& description() const { return _description; }
    [[nodiscard]] WorldNode* node() const { return _node; }
    [[nodiscard]] bool special() const { return _map_ids.empty(); }
    [[nodiscard]] bool small_textbox() const { return _small_textbox; }

    [[nodiscard]] std::string text() const;
    void text(const std::string& text) { _text = text; }
    void apply_text(World& world);

    [[nodiscard]] const std::vector<uint16_t>& text_ids() const { return _text_ids; }
    void text_id(uint16_t text_id) { _text_ids = { text_id }; }
    void text_ids(const std::vector<uint16_t>& text_ids) { _text_ids = text_ids; }

    [[nodiscard]] const std::vector<uint16_t>& map_ids() const { return _map_ids; }
    [[nodiscard]] Position position() const { return _position; }
    [[nodiscard]] uint8_t orientation() const { return _orientation; }
    [[nodiscard]] bool high_palette() const { return _high_palette; }

    [[nodiscard]] virtual Json to_json() const;
    [[nodiscard]] static HintSource* from_json(const Json& json, const std::map<std::string, WorldNode*>& nodes);
};
