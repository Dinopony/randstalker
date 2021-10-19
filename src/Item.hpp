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

    void setAllowedOnGround(bool allowed) { _isAllowedOnGround = allowed; }
    bool isAllowedOnGround() const { return _isAllowedOnGround; }
    
    uint16_t getGoldWorth() { return _goldWorth; }
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
    ItemGolds(uint8_t itemID, uint8_t goldWorth) :
        Item(itemID, "Golds", 0, false)
    {
        this->setGoldWorth(goldWorth);
    }

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
        // Do nothing, since it is not a real item
        return;
    }
};