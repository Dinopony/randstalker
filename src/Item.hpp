#pragma once

#include "MegadriveTools/MdRom.hpp"
#include "Enums/ItemCodes.hpp"
#include <sstream>

class Item
{
public:
    Item(uint8_t itemID, const std::string& name, uint16_t goldWorth, bool isAllowedOnGround = true) :
        _itemID(itemID),
        _name(name),
        _goldWorth(goldWorth),
        _isAllowedOnGround(isAllowedOnGround)
    {}

    virtual ~Item() {}

    void setID(uint8_t id) { _itemID = id; }
    uint8_t getID() const { return _itemID; }

    void setName(const std::string& name) { _name = name; }
    const std::string& getName() const { return _name; }
    bool isAllowedOnGround() const { return _isAllowedOnGround; }
    virtual void setGoldWorth(uint16_t goldWorth) { _goldWorth = goldWorth; }

    virtual void writeToROM(md::ROM& rom) const
    {
        rom.setWord(0x029306 + _itemID * 0x04, _goldWorth);
    }

protected:
    uint8_t _itemID;
    std::string _name;
    uint16_t _goldWorth;
    bool _isAllowedOnGround;
};

////////////////////////////////////////////////////////////////

class ItemGolds : public Item
{
public:
    ItemGolds(uint8_t itemID) :
        Item(itemID, "1 gold", 1, false)
    {}

    virtual ~ItemGolds() {}

    virtual void setGoldWorth(uint16_t goldWorth)
    {
        Item::setGoldWorth(goldWorth);

        std::ostringstream goldName;
		goldName << (uint32_t)goldWorth << " gold";
        if(goldWorth > 1)
            goldName << "s";
        this->setName(goldName.str());
    }

    virtual void writeToROM(md::ROM& rom) const
    {
        uint32_t baseGoldValuesAddress = rom.getStoredAddress("data_gold_values");
        rom.setByte(baseGoldValuesAddress + (_itemID - ITEM_GOLDS_START), (uint8_t)_goldWorth);
    }
};