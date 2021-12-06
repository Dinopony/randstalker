#pragma once

#include <json.hpp>
#include <sstream>

class Item
{
private:
    uint8_t     _id;
    std::string _name;
    uint8_t     _max_quantity;
    uint8_t     _starting_quantity;
    uint16_t    _gold_value;
    bool        _allowed_on_ground;

public:
    Item() :
        _id(0xFF),
        _name(),
        _max_quantity(0),
        _starting_quantity(0),
        _gold_value(0),
        _allowed_on_ground(false)
    {}

    Item(uint8_t id, const std::string& name, uint8_t max_quantity, uint8_t starting_quantity, uint16_t gold_value, bool allowed_on_ground = true) :
        _id                 (id),
        _name               (name),
        _max_quantity       (max_quantity),
        _starting_quantity  (starting_quantity),
        _gold_value         (gold_value),
        _allowed_on_ground  (allowed_on_ground)
    {}
    
    [[nodiscard]] uint8_t id() const { return _id; }
    Item& id(uint8_t id) { _id = id; return *this; }

    [[nodiscard]] const std::string& name() const { return _name; }
    Item& name(const std::string& name) { _name = name; return *this; }

    [[nodiscard]] uint8_t starting_quantity() const { return std::min(_starting_quantity, _max_quantity); }
    Item& starting_quantity(uint8_t quantity) { _starting_quantity = quantity; return *this; }
    
    [[nodiscard]] uint8_t max_quantity() const { return _max_quantity; }
    Item& max_quantity(uint8_t quantity) { _max_quantity = quantity; return *this; }

    [[nodiscard]] uint16_t gold_value() const { return _gold_value; }
    virtual Item& gold_value(uint16_t value) { _gold_value = value; return *this; }

    [[nodiscard]] bool allowed_on_ground() const { return _allowed_on_ground; }
    void allowed_on_ground(bool allowed) { _allowed_on_ground = allowed; }

    [[nodiscard]] Json to_json() const;
    static Item* from_json(uint8_t id, const Json& json);
    void apply_json(const Json& json);
};

////////////////////////////////////////////////////////////////

class ItemGolds : public Item
{
public:
    ItemGolds(uint8_t id, uint16_t gold_value) : 
        Item(id,
             std::to_string(gold_value) + " golds",
             0,
             0,
             gold_value,
             false)
    {}

    Item& gold_value(uint16_t value) override
    {
        this->name(std::to_string(value) + " golds");
        return Item::gold_value(value);
    }
};
