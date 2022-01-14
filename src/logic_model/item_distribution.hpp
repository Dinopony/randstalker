#pragma once

#include <cstdint>
#include <landstalker_lib/tools/json.hpp>

class ItemDistribution
{
private:
    uint16_t _quantity = 0;
    bool _allowed_on_ground = true;

public:
    ItemDistribution() = default;

    ItemDistribution(uint16_t quantity, bool allowed_on_ground) :
        _quantity           (quantity),
        _allowed_on_ground  (allowed_on_ground)
    {}

    [[nodiscard]] uint16_t quantity() const { return _quantity; }
    void quantity(uint16_t value) { _quantity = value; }
    void add(uint8_t quantity) { _quantity += quantity; }
    void remove(uint8_t quantity)
    {
        if(_quantity <= quantity)
            _quantity = 0;
        else
            _quantity -= quantity;
    }

    [[nodiscard]] bool allowed_on_ground() const { return _allowed_on_ground; }
    void allowed_on_ground(bool allowed) { _allowed_on_ground = allowed; }

    [[nodiscard]] Json to_json() const
    {
        return {
            {"quantity", _quantity},
            {"allowedOnGround", _allowed_on_ground}
        };
    }

    static ItemDistribution* from_json(const Json& json)
    {
        uint16_t quantity = json.value("quantity", 0);
        bool allowed_on_ground = json.value("allowedOnGround", true);

        return new ItemDistribution(quantity, allowed_on_ground);
    }
};

