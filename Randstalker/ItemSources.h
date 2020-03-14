#pragma once

#include <string>
#include "Item.h"

class WorldRegion;

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemSource
{
public:
    ItemSource(const std::string& name) :
        _name(name),
        _item(nullptr),
        _region(nullptr)
    {}

    const std::string& getName() const { return _name; }

    void setItem(Item* item) { _item = item; }
    Item* getItem() const { return _item; }
    uint8_t getItemID() const { return (_item) ? _item->getID() : ITEM_NONE; }

    void addHint(const std::string& hint) { _hints.push_back(hint); }
    const std::vector<std::string>& getHints() const { return _hints; }

    void setRegion(WorldRegion* region) { _region = region; }
    WorldRegion* getRegion() const { return _region; }

    virtual bool isItemCompatible(Item* item) const = 0;
    virtual void writeToROM(md::ROM& rom) const = 0;

private:
    std::string _name;
    Item* _item;

    std::vector<std::string> _hints;
    WorldRegion* _region;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemChest : public ItemSource
{
public:
    ItemChest(uint8_t chestID, const std::string& name) :
        ItemSource(name),
        _chestID(chestID)
    {}

    uint8_t getID() const { return _chestID; }
    virtual bool isItemCompatible(Item* item) const { return true; }

    virtual void writeToROM(md::ROM& rom) const
    {
        rom.setByte(0x9EABE + _chestID, this->getItemID());
    }

private:
    uint8_t _chestID;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemOnGround : public ItemSource
{
public:
    ItemOnGround(uint32_t addressInROM, const std::string& name) :
        ItemSource(name), _addressesInROM({ addressInROM })
    {}

    ItemOnGround(std::vector<uint32_t> addressesInROM, const std::string& name) :
        ItemSource(name), _addressesInROM(addressesInROM)
    {}

    virtual bool isItemCompatible(Item* item) const
    {
        return !item || item->isAllowedOnGround();
    }

    virtual void writeToROM(md::ROM& rom) const
    {
        for (uint32_t address : _addressesInROM)
            rom.setByte(address, this->getItemID() + 0xC0);
    }

private:
    std::vector<uint32_t> _addressesInROM;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemReward : public ItemSource
{
public:
    ItemReward(uint32_t addressInROM, const std::string& name) :
        ItemSource(name), _addressInROM(addressInROM)
    {
        this->addHint("owned by someone willing to give it to the brave");
    }

    virtual bool isItemCompatible(Item* item) const
    {
        return (item->getID() < ITEM_NONE);
    }

    virtual void writeToROM(md::ROM& rom) const
    {
        rom.setByte(_addressInROM, this->getItemID());
    }

private:
    uint32_t _addressInROM;
};

///////////////////////////////////////////////////////////////////////////////////////////////

/// Helper class used for making coherent shops without item duplicates
class ItemShop
{
public:
    ItemShop() :
        _itemSourcesInShop(),
        _itemValueMulitplier(1.f)
    {}

    void addItemSource(ItemSource* itemSource) { _itemSourcesInShop.push_back(itemSource); }

    bool isItemAlreadyInShop(Item* item)
    {
        for (ItemSource* source : _itemSourcesInShop)
            if (source->getItem() == item)
                return true;
        return false;
    }

private:
    float _itemValueMulitplier;
    std::vector<ItemSource*> _itemSourcesInShop;
};

// --------------------------------------------------------------------------

/// Actual item source class for shop items
class ItemInShop : public ItemSource
{
public:
    ItemInShop(uint32_t addressInROM, const std::string& name, ItemShop* shop) :
        ItemSource(name),
        _addressesInROM({ addressInROM }),
        _shop(shop)
    {
        if (_shop)
            _shop->addItemSource(this);

        this->addHint("owned by someone trying to make profit out of it");
    }

    ItemInShop(std::vector<uint32_t> addressesInROM, const std::string& name, ItemShop* shop) :
        ItemSource(name),
        _addressesInROM(addressesInROM),
        _shop(shop)
    {
        if (_shop)
            _shop->addItemSource(this);

        this->addHint("owned by someone trying to make profit out of it");
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

    virtual void writeToROM(md::ROM& rom) const
    {
        for (uint32_t address : _addressesInROM)
            rom.setByte(address, this->getItemID() + 0xC0);
    }

private:
    std::vector<uint32_t> _addressesInROM;
    ItemShop* _shop;
};