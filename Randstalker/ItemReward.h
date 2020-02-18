#pragma once

#include "AbstractItemSource.h"

class ItemReward : public AbstractItemSource
{
public:
    ItemReward(uint32_t addressInROM, const std::string& name) :
        AbstractItemSource(name),
        _addressInROM(addressInROM)
    {
        this->addHint("owned by someone willing to give it to the brave");
    }

    virtual bool isItemCompatible(Item* item) const
    {
        return (item->getID() < ITEM_NONE);
    }

    virtual void writeToROM(GameROM& rom) const
    {
        rom.setByte(_addressInROM, this->getItemID());
    }

private:
    uint32_t _addressInROM;
};
