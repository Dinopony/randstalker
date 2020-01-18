#pragma once

#include "GameROM.h"

class Item
{
public:
    Item(uint8_t itemID, const std::string& name, uint16_t priceInShop, bool isAllowedForPedestals = false) :
        _itemID(itemID),
        _name(name),
        _priceInShop(priceInShop),
        _isAllowedForPedestals(isAllowedForPedestals)
    {}

    uint8_t getID() const { return _itemID; }
    bool isAllowedForPedestals() const { return _isAllowedForPedestals; }
    const std::string& getName() const { return _name; }

    void writeToROM(GameROM& rom) const
    {
        rom.setWord(0x029306 + _itemID * 0x04, _priceInShop);
    }

private:
    uint8_t _itemID;
    std::string _name;
    uint16_t _priceInShop;
    bool _isAllowedForPedestals;
};