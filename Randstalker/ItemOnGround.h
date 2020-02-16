#pragma once

#include <vector>
#include "AbstractItemSource.h"

class ItemOnGround : public AbstractItemSource
{
public:
    ItemOnGround(uint32_t addressInROM, const std::string& name) :
        AbstractItemSource(name),
        _addressesInROM()
    {
        _addressesInROM.push_back(addressInROM);
    }

    ItemOnGround(std::vector<uint32_t> addressesInROM, const std::string& name) :
        AbstractItemSource(name),
        _addressesInROM(addressesInROM)
    {}

    void addOtherAddress(uint32_t addressInROM)
    {
        _addressesInROM.push_back(addressInROM);
    }

    virtual bool isItemCompatible(Item* item) const
    {
        return !item || item->isAllowedOnGround();
    }

    virtual void writeToROM(GameROM& rom) const
    {
        for (uint32_t address : _addressesInROM)
            rom.setByte(address, this->getItemID() + 0xC0);
    }

private:
    std::vector<uint32_t> _addressesInROM;
};