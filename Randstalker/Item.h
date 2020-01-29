#pragma once

#include "GameROM.h"

class Item
{
public:
    Item(uint8_t itemID, const std::string& name, uint16_t priceInShop, bool isAllowedOnGround = false) :
        _itemID(itemID),
        _name(name),
        _priceInShop(priceInShop),
        _isAllowedOnGround(isAllowedOnGround)
    {}

    void setID(uint8_t id) { _itemID = id; }
    uint8_t getID() const { return _itemID; }

    void setName(const std::string& name) { _name = name; }
    const std::string& getName() const { return _name; }

    void setPrice(uint16_t priceInShop) { _priceInShop = priceInShop; }

    bool isAllowedOnGround() const { return _isAllowedOnGround; }

    void writeToROM(GameROM& rom) const
    {
        rom.setWord(0x029306 + _itemID * 0x04, _priceInShop);
    }

private:
    uint8_t _itemID;
    std::string _name;
    uint16_t _priceInShop;
    bool _isAllowedOnGround;
};