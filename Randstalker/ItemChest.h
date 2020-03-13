#pragma once

#include "AbstractItemSource.h"

class ItemChest : public AbstractItemSource
{
public:
    ItemChest(uint8_t chestID, const std::string& name) :
        AbstractItemSource(name),
        _chestID(chestID)
    {}

    uint8_t getID() const { return _chestID; }

    virtual bool isItemCompatible(Item* item) const
    {
        return true;
    }

    virtual void writeToROM(md::ROM& rom) const
    {
        rom.setByte(0x9EABE + _chestID, this->getItemID());
    }

private:
    uint8_t _chestID;
};