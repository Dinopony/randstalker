#pragma once

#include <cstdint>
#include <utility>
#include <landstalker_lib/tools/json.hpp>
#include <landstalker_lib/tools/color.hpp>

class ArgumentDictionary;

class PersonalSettings
{
private:
    bool _add_ingame_item_tracker = false;
    Color _hud_color = Color(0x40, 0x20, 0x80);
    std::pair<Color, Color> _nigel_colors = std::make_pair(Color(0x00, 0xA0, 0x80), Color(0x00, 0x40, 0x20));
    bool _remove_music = false;

public:
    explicit PersonalSettings(const ArgumentDictionary& args);

    // Personal options
    [[nodiscard]] bool add_ingame_item_tracker() const { return _add_ingame_item_tracker; }
    [[nodiscard]] Color hud_color() const { return _hud_color; }
    [[nodiscard]] const std::pair<Color, Color>& nigel_colors() const { return _nigel_colors; }
    [[nodiscard]] bool remove_music() const { return _remove_music; }

    void parse_json(const Json& json);
};
