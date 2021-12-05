#pragma once

#include <array>
#include <cstdint>
#include <json.hpp>

constexpr size_t COLOR_COUNT = 13;

class MapPalette
{
public:
    struct Color {
        uint8_t r;
        uint8_t g;
        uint8_t b;

        Color();
        Color(uint8_t r, uint8_t g, uint8_t b);
        Color(uint16_t color_word);

        void apply_factor(float factor);
        void desaturate(float factor);
        
        uint16_t to_word() const;
        Json to_json() const;
    };

private:
    std::array<Color, COLOR_COUNT> _colors;

public:
    MapPalette() {}
    MapPalette(const std::array<uint16_t, COLOR_COUNT>& color_words);

    const std::array<Color, COLOR_COUNT>& colors() const { return _colors; }
    std::array<Color, COLOR_COUNT>& colors() { return _colors; }

    void apply_factor(float factor);

    Color& color(uint8_t id) { return _colors[id]; }
    void color(uint8_t id, Color c) { _colors[id] = c; }

    Json to_json() const;
};
