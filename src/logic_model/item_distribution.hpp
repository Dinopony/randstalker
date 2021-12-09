#pragma once

#include <cstdint>
#include <landstalker_lib/tools/json.hpp>

class ItemDistribution
{
private:
    /// The quantity placed before the beginning of the seed, in a fully random manner (no logic taken in account)
    uint16_t _mandatory_quantity = 0;
    /// The quantity placed when a path requiring it is opened by the exploration algorithm (only once per generation)
    uint16_t _key_quantity = 1;
    /// The quantity placed at the end of generation to randomly fill remaining item sources
    uint16_t _filler_quantity = 0;

public:
    ItemDistribution() = default;

    ItemDistribution(uint16_t mandatory_quantity, uint16_t key_quantity, uint16_t filler_quantity) :
        _mandatory_quantity (mandatory_quantity),
        _key_quantity       (key_quantity),
        _filler_quantity    (filler_quantity)
    {}

    [[nodiscard]] uint16_t mandatory_quantity() const { return _mandatory_quantity; }
    void mandatory_quantity(uint16_t value) { _mandatory_quantity = value; }

    [[nodiscard]] uint16_t key_quantity() const { return _key_quantity; }
    void key_quantity(uint16_t value) { _key_quantity = value; }

    [[nodiscard]] uint16_t filler_quantity() const { return _filler_quantity; }
    void filler_quantity(uint16_t value) { _filler_quantity = value; }

    [[nodiscard]] Json to_json() const
    {
        return {
            {"mandatoryQuantity", _mandatory_quantity},
            {"keyQuantity", _key_quantity},
            {"fillerQuantity", _filler_quantity},
        };
    }

    static ItemDistribution* from_json(const Json& json)
    {
        uint16_t mandatory_quantity = json.value("mandatoryQuantity", 0);
        uint16_t key_quantity = json.value("keyQuantity", 0);
        uint16_t filler_quantity = json.value("fillerQuantity", 0);

        return new ItemDistribution(mandatory_quantity, key_quantity, filler_quantity);
    }
};

