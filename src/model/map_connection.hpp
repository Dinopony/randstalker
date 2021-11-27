#pragma once

#include "../extlibs/json.hpp"

struct MapConnection 
{
    uint16_t map_id;
    uint8_t pos_x;
    uint8_t pos_y;
    uint8_t extra_byte;

    uint16_t destination_map_id;
    uint8_t destination_pos_x;
    uint8_t destination_pos_y;
    uint8_t destination_extra_byte;

    /// Extra byte:
    /// 0x1 => Initially closed?
    /// 0x2 => NE
    /// 0x4 => NW
    /// 0x8 => ???
    /// 0x10 => NE Stairs
    /// 0x20 => NW Stairs

    Json to_json() const {
        Json json;

        json["mapId"] = map_id;
        json["posX"] = pos_x;
        json["posY"] = pos_y;
        json["extraByte"] = extra_byte;

        json["destinationMapId"] = destination_map_id;
        json["destinationX"] = destination_pos_x;
        json["destinationY"] = destination_pos_y;
        json["destinationExtraByte"] = destination_extra_byte;

        return json;
    }
};
