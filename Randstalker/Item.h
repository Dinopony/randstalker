#pragma once

#include "MegadriveTools/MdRom.h"
#include "ItemCodes.h"

class Item
{
public:
    Item(uint8_t itemID, const std::string& name, uint16_t goldWorth, bool isAllowedOnGround = false) :
        _itemID(itemID),
        _name(name),
        _goldWorth(goldWorth),
        _isAllowedOnGround(isAllowedOnGround)
    {}

    void setID(uint8_t id) { _itemID = id; }
    uint8_t getID() const { return _itemID; }

    void setName(const std::string& name) { _name = name; }
    const std::string& getName() const { return _name; }

    void setGoldWorth(uint16_t goldWorth) { _goldWorth = goldWorth; }

    bool isAllowedOnGround() const { return _isAllowedOnGround; }

    void writeToROM(md::ROM& rom) const
    {
        if (_itemID < ITEM_GOLDS_START)
        {
            rom.setWord(0x029306 + _itemID * 0x04, _goldWorth);
        }
        else
        {
            uint32_t baseGoldValuesAddress = rom.getStoredAddress("data_gold_values");
            rom.setByte(baseGoldValuesAddress + (_itemID - ITEM_GOLDS_START), (uint8_t)_goldWorth);
        }
    }

private:
    uint8_t _itemID;
    std::string _name;
    uint16_t _goldWorth;
    bool _isAllowedOnGround;
};