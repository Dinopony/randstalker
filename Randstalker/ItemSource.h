#pragma once

#include <cstdint>
#include <vector>
#include "GameROM.h"
#include "Constants/ItemCodes.h"
#include "Item.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class AbstractItemSource
{
public:
    AbstractItemSource(const std::string& name) :
        _name   (name),
        _item   (nullptr)
    {}

    const std::string& getName() const { return _name; }

    void setItem(Item* item) { _item = item; }
    Item* getItem() const { return _item;  }
    uint8_t getItemID() const { return (_item) ? _item->getID() : ITEM_NONE; }

    virtual bool isItemCompatible(Item* item) const = 0;
    virtual void writeToROM(GameROM& rom) const = 0;

private:
    std::string _name;
    Item* _item;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ItemChest : public AbstractItemSource
{
public:
    ItemChest(uint8_t chestID, uint8_t defaultItemID, const std::string& name) :
        AbstractItemSource(name),
        _chestID(chestID)
    {}

    uint8_t getID() const { return _chestID; }

    virtual bool isItemCompatible(Item* item) const
    {
        return true;
    }

    virtual void writeToROM(GameROM& rom) const
    {
        rom.setByte(0x9EABE + _chestID, this->getItemID());
    }

private:
    uint8_t _chestID;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ItemOnGround : public AbstractItemSource
{
public:
    ItemOnGround(uint32_t addressInROM, const std::string& name, bool isShop = false, bool canCarryLifestock = false) :
        AbstractItemSource(name),
        _addressesInROM(),
        _isShop(isShop),
        _canCarryLifestock(canCarryLifestock)
    {
        _addressesInROM.push_back(addressInROM);
    }

    ItemOnGround(std::vector<uint32_t> addressesInROM, const std::string& name, bool isShop = false, bool canCarryLifestock = false) :
        AbstractItemSource(name),
        _addressesInROM(addressesInROM),
        _isShop(isShop),
        _canCarryLifestock(canCarryLifestock)
    {}

    void addOtherAddress(uint32_t addressInROM)
    {
        _addressesInROM.push_back(addressInROM);
    }

    virtual bool isItemCompatible(Item* item) const
    { 
        if (!item)
            return true;

        if (_isShop)
        {
            uint8_t itemID = item->getID();
            if (itemID == ITEM_5_GOLDS || itemID == ITEM_20_GOLDS || itemID == ITEM_50_GOLDS || itemID == ITEM_200_GOLDS || itemID == ITEM_NONE)
                return false;
            if (itemID == ITEM_LIFESTOCK && !_canCarryLifestock)
                return false;
            return true;
        }

        return item->isAllowedOnGround(); 
    }

    virtual void writeToROM(GameROM& rom) const
    {
        for(uint32_t address : _addressesInROM)
            rom.setByte(address, this->getItemID() + 0xC0);
    }

private:
    std::vector<uint32_t> _addressesInROM;
    bool _isShop;
    bool _canCarryLifestock;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ItemReward : public AbstractItemSource
{
public:
    ItemReward(uint32_t addressInROM, const std::string& name) :
        AbstractItemSource(name),
        _addressInROM(addressInROM)
    {}

    virtual bool isItemCompatible(Item* item) const
    {
        uint8_t itemID = item->getID();
        return (itemID != ITEM_5_GOLDS && itemID != ITEM_20_GOLDS && itemID != ITEM_50_GOLDS && itemID != ITEM_200_GOLDS);
    }

    virtual void writeToROM(GameROM& rom) const
    {
        rom.setByte(_addressInROM, this->getItemID());
    }

private:
    uint32_t _addressInROM;
};
