#include "map_palette.hpp"
#include "../world_model/world.hpp"
#include "../exceptions.hpp"

MapPalette::Color::Color() : r(0), g(0), b(0)
{}

MapPalette::Color::Color(uint8_t ir, uint8_t ig, uint8_t ib) : r(ir), g(ig), b(ib)
{}

MapPalette::Color::Color(uint16_t color_word)
{
    r = color_word & 0xF;
    g = (color_word >> 4) & 0xF;
    b = (color_word >> 8) & 0xF;         
}

void MapPalette::Color::apply_factor(float factor)
{
    float float_r = r * factor;
    r = (uint8_t)std::round(std::min(float_r, (float)0xF));

    float float_g = g * factor;
    g = (uint8_t)std::round(std::min(float_g, (float)0xF));

    float float_b = b * factor;
    b = (uint8_t)std::round(std::min(float_b, (float)0xF));
}

void MapPalette::Color::desaturate(float factor)
{
    float gray_equivalent = (r + g + b) / 3.f;

    float float_r = (r * (1-factor)) + (gray_equivalent * factor);
    r = (uint8_t)std::round(std::min(float_r, (float)0xF));

    float float_g = (g * (1-factor)) + (gray_equivalent * factor);
    g = (uint8_t)std::round(std::min(float_g, (float)0xF));

    float float_b = (b * (1-factor)) + (gray_equivalent * factor);
    b = (uint8_t)std::round(std::min(float_b, (float)0xF));
}

uint16_t MapPalette::Color::to_word() const
{
    uint16_t color_word = 0;
    color_word |= r & 0xF;
    color_word |= (g & 0xF) << 4;
    color_word |= (b & 0xF) << 8;
    return color_word;
}

Json MapPalette::Color::to_json() const 
{
    return Json({
        { "r", r },
        { "g", g },
        { "b", b }
    });
}

/////////////////////////////////////////////////////

MapPalette::MapPalette(const std::array<uint16_t, COLOR_COUNT>& color_words)
{
    for(uint8_t i=0 ; i<COLOR_COUNT ; ++i)
        _colors[i] = Color(color_words[i]);
}

void MapPalette::apply_factor(float factor)
{
    for(uint8_t id=0 ; id<COLOR_COUNT ; ++id)
        _colors[id].apply_factor(factor);
}

Json MapPalette::to_json() const
{
    Json json = Json::array();
    for(uint8_t i=0 ; i<COLOR_COUNT ; ++i)
        json.push_back(_colors[i].to_json());
    return json;
}
