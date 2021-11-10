#pragma once

#include <cstdint>
#include "../extlibs/json.hpp"

struct Flag
{
    uint16_t byte;
    uint8_t bit;

    Flag() : byte(0), bit(0)
    {}

    Flag(uint16_t byte, uint8_t bit) : byte(byte), bit(bit)
    {}

    Json to_json() const
    {
        Json json;
        json["flagByte"] = byte;
        json["flagBit"] = bit;
        return json;
    }
};