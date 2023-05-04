#pragma once

#include <cstdint>
#include <landstalker-lib/tools/json.hpp>

class ItemDistribution
{
private:
    uint16_t _quantity = 0;

public:
    ItemDistribution() = default;

    explicit ItemDistribution(uint16_t quantity) : _quantity (quantity) {}

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

    [[nodiscard]] Json to_json() const
    {
        return {
            {"quantity", _quantity}
        };
    }

    static ItemDistribution* from_json(const Json& json)
    {
        uint16_t quantity = json.value("quantity", 0);
        return new (quantity);
    }
};

