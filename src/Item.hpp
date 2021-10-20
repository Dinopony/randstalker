#pragma once

#include "MegadriveTools/MdRom.hpp"
#include "Enums/ItemCodes.hpp"
#include <sstream>

class Item
{
public:
    Item(uint8_t itemID, const std::string& name, uint8_t maxQuantity, uint16_t goldWorth, bool isAllowedOnGround = true) :
        _itemID             (itemID),
        _name               (name),
        _startingQuantity   (0),
        _maxQuantity        (maxQuantity),
        _goldWorth          (goldWorth),
        _isAllowedOnGround  (isAllowedOnGround)
    {}

    virtual ~Item() {}

    void setID(uint8_t id) { _itemID = id; }
    uint8_t getID() const { return _itemID; }

    void setName(const std::string& name) { _name = name; }
    const std::string& getName() const { return _name; }
    
    uint8_t getStartingQuantity() const { return _startingQuantity; }
    void setStartingQuantity(uint8_t quantity) { _startingQuantity = quantity; }
    
    uint8_t getMaxQuantity() const { return _maxQuantity; }
    void setMaxQuantity(uint8_t quantity) { _maxQuantity = quantity; }

    uint16_t getGoldWorth() { return _goldWorth; }
    virtual void setGoldWorth(uint16_t goldWorth) { _goldWorth = goldWorth; }

    void setAllowedOnGround(bool allowed) { _isAllowedOnGround = allowed; }
    bool isAllowedOnGround() const { return _isAllowedOnGround; }

    virtual void writeToROM(md::ROM& rom) const
    {
        constexpr uint32_t itemInfoTableBaseAddr = 0x029304;
        uint32_t thisItemBaseAddr = itemInfoTableBaseAddr + _itemID * 0x04;

        // Set the max quantity
        uint8_t verbAndMaxQuantity = rom.getByte(thisItemBaseAddr);
        verbAndMaxQuantity &= 0xF0; // Keep the verb but erase the default quantity
        verbAndMaxQuantity += _maxQuantity;
        rom.setByte(thisItemBaseAddr, verbAndMaxQuantity);

        // Set gold value
        rom.setWord(thisItemBaseAddr + 0x2, _goldWorth);
    }

protected:
    uint8_t _itemID;

    std::string _name;
    uint8_t _maxQuantity;
    uint8_t _startingQuantity;
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
		goldName << (uint32_t)goldWorth << " golds";
        this->setName(goldName.str());
    }

    virtual void writeToROM(md::ROM& rom) const
    {
        // Do nothing, since it is not a real item
        return;
    }
};