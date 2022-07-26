#pragma once

#include <cstdint>
#include <utility>
#include <landstalker_lib/tools/json.hpp>
#include <landstalker_lib/tools/color.hpp>
#include <landstalker_lib/constants/values.hpp>

class ArgumentDictionary;

class PersonalSettings
{
private:
    std::array<std::string, ITEM_COUNT+1> _item_names;

    bool _add_ingame_item_tracker = false;
    Color _hud_color = Color(0x40, 0x20, 0x80);
    std::pair<Color, Color> _nigel_colors = std::make_pair(Color(0x00, 0xA0, 0x80), Color(0x00, 0x40, 0x20));
    bool _remove_music = false;
    bool _swap_overworld_music = false;
    std::array<uint8_t, 40> _inventory_order;

public:
    explicit PersonalSettings(const ArgumentDictionary& args, const std::array<std::string, ITEM_COUNT+1>& item_names);

    // Personal options
    [[nodiscard]] bool add_ingame_item_tracker() const { return _add_ingame_item_tracker; }
    [[nodiscard]] Color hud_color() const { return _hud_color; }
    [[nodiscard]] const std::pair<Color, Color>& nigel_colors() const { return _nigel_colors; }
    [[nodiscard]] bool remove_music() const { return _remove_music; }
    [[nodiscard]] bool swap_overworld_music() const { return _swap_overworld_music; }
    [[nodiscard]] const std::array<uint8_t, 40>& inventory_order() const { return _inventory_order; }

    void parse_json(const Json& json);
};
