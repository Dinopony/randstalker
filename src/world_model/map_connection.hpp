#pragma once

#include <json.hpp>

class MapConnection 
{
private:
    uint16_t _map_id_1;
    uint8_t _pos_x_1;
    uint8_t _pos_y_1;
    uint8_t _extra_byte_1;

    uint16_t _map_id_2;
    uint8_t _pos_x_2;
    uint8_t _pos_y_2;
    uint8_t _extra_byte_2;

    /// Extra byte:
    /// 0x1 => Initially closed?
    /// 0x2 => NE
    /// 0x4 => NW
    /// 0x8 => ???
    /// 0x10 => NE Stairs
    /// 0x20 => NW Stairs

public:
    MapConnection() : 
        _map_id_1        (0),
        _pos_x_1         (0),
        _pos_y_1         (0),
        _extra_byte_1    (0),
        _map_id_2        (0),
        _pos_x_2         (0),
        _pos_y_2         (0),
        _extra_byte_2    (0)
    {}

    MapConnection(uint16_t map_id_1, uint16_t map_id_2, 
                  uint8_t pos_x_1, uint8_t pos_y_1, uint8_t pos_x_2, uint8_t pos_y_2, 
                  uint8_t extra_byte_1 = 0, uint8_t extra_byte_2 = 0) : 
        _map_id_1        (map_id_1),
        _pos_x_1         (pos_x_1),
        _pos_y_1         (pos_y_1),
        _extra_byte_1    (extra_byte_1),
        _map_id_2        (map_id_2),
        _pos_x_2         (pos_x_2),
        _pos_y_2         (pos_y_2),
        _extra_byte_2    (extra_byte_2)
    {}

    uint16_t map_id_1() const { return _map_id_1; }
    void map_id_1(uint16_t value) { _map_id_1 = value; }

    uint8_t pos_x_1() const { return _pos_x_1; }
    void pos_x_1(uint8_t value) { _pos_x_1 = value; }

    uint8_t pos_y_1() const { return _pos_y_1; }
    void pos_y_1(uint8_t value) { _pos_y_1 = value; }

    uint8_t extra_byte_1() const { return _extra_byte_1; }
    void extra_byte_1(uint8_t value) { _extra_byte_1 = value; }

    uint16_t map_id_2() const { return _map_id_2; }
    void map_id_2(uint16_t value) { _map_id_2 = value; }

    uint8_t pos_x_2() const { return _pos_x_2; }
    void pos_x_2(uint8_t value) { _pos_x_2 = value; }

    uint8_t pos_y_2() const { return _pos_y_2; }
    void pos_y_2(uint8_t value) { _pos_y_2 = value; }

    uint8_t extra_byte_2() const { return _extra_byte_2; }
    void extra_byte_2(uint8_t value) { _extra_byte_2 = value; }
    
    bool check_maps(uint16_t map_id_1, uint16_t map_id_2) const
    {
        if(_map_id_1 == map_id_1 && _map_id_2 == map_id_2)
            return true;
        if(_map_id_1 == map_id_2 && _map_id_2 == map_id_1)
            return true;
        return false;
    }

    Json to_json() const
    {
        Json json;

        json["mapId1"] = _map_id_1;
        json["posX1"] = _pos_x_1;
        json["posY1"] = _pos_y_1;
        json["extraByte1"] = _extra_byte_1;

        json["mapId2"] = _map_id_2;
        json["posX2"] = _pos_x_2;
        json["posY2"] = _pos_y_2;
        json["extraByte2"] = _extra_byte_2;

        return json;
    }
};
