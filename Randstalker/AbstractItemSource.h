#pragma once

#include "Item.h"

class AbstractItemSource
{
public:
    AbstractItemSource(const std::string& name) :
        _name(name),
        _item(nullptr)
    {}

    const std::string& getName() const { return _name; }

    void setItem(Item* item) { _item = item; }
    Item* getItem() const { return _item; }
    uint8_t getItemID() const { return (_item) ? _item->getID() : ITEM_NONE; }

    virtual bool isItemCompatible(Item* item) const = 0;
    virtual void writeToROM(GameROM& rom) const = 0;

private:
    std::string _name;
    Item* _item;
};