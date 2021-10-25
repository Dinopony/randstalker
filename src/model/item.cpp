#include "item.hpp"

void Item::write_to_rom(md::ROM& rom) const
{
    constexpr uint32_t ITEM_TABLE_BASE_ADDR = 0x029304;
    uint32_t item_base_addr = ITEM_TABLE_BASE_ADDR + _id * 0x04;

    // Set the max quantity
    uint8_t verb_and_max_qty = rom.getByte(item_base_addr);
    verb_and_max_qty &= 0xF0; // Keep the verb but erase the default quantity
    verb_and_max_qty += _max_quantity;
    rom.setByte(item_base_addr, verb_and_max_qty);

    // Set gold value
    rom.setWord(item_base_addr + 0x2, _gold_value);
}

Json Item::to_json() const
{
    Json json;
    json["id"] = _id;
    json["name"] = _name;
    json["maxQuantity"] = _max_quantity;
    json["startingQuantity"] = _starting_quantity;
    json["goldValue"] = _gold_value;
    json["allowedOnGround"] = _allowed_on_ground;
    return json;
}

Item* Item::from_json(const Json& json)
{
    uint8_t id = json.at("id");
    const std::string& name = json.at("name");
    uint8_t max_quantity = json.value("maxQuantity", 1);
    uint8_t starting_quantity = json.value("startingQuantity", 0);
    uint16_t gold_value = json.value("goldValue", 0);
    bool allowed_on_ground = json.value("allowedOnGround", true);

    return new Item(id, name, max_quantity, gold_value, allowed_on_ground);
}