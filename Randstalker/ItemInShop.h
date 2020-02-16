#pragma once

#include <vector>
#include "AbstractItemSource.h"
#include "ItemShop.h"

class ItemInShop : public AbstractItemSource
{
public:
    ItemInShop(uint32_t addressInROM, const std::string& name, ItemShop* shop) :
        AbstractItemSource(name),
        _addressesInROM(),
        _shop(shop)
    {
        _addressesInROM.push_back(addressInROM);
        if (_shop)
            _shop->addItemSource(this);
    }

    ItemInShop(std::vector<uint32_t> addressesInROM, const std::string& name, ItemShop* shop) :
        AbstractItemSource(name),
        _addressesInROM(addressesInROM),
        _shop(shop)
    {
        if (_shop)
            _shop->addItemSource(this);
    }

    void addOtherAddress(uint32_t addressInROM)
    {
        _addressesInROM.push_back(addressInROM);
    }

    virtual bool isItemCompatible(Item* item) const
    {
        if (!item)
            return true;

        uint8_t itemID = item->getID();
        if (itemID == ITEM_NONE || itemID >= ITEM_GOLDS_START)
            return false;

        if (_shop && _shop->isItemAlreadyInShop(item))
            return false;

        return true;
    }

    virtual void writeToROM(GameROM& rom) const
    {
        for (uint32_t address : _addressesInROM)
            rom.setByte(address, this->getItemID() + 0xC0);
    }

private:
    std::vector<uint32_t> _addressesInROM;
    ItemShop* _shop;
};