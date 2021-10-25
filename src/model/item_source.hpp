#pragma once

#include <string>
#include "item.hpp"

class WorldRegion;

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemSource
{
private:
    std::string _name;
    Item* _item;
    WorldRegion* _region;
    std::vector<std::string> _hints;

public:
    ItemSource(const std::string& name, WorldRegion* region, const std::vector<std::string>& hints);

    virtual std::string type_name() const = 0;

    const std::string& name() const { return _name; }
    ItemSource& name(const std::string& name) { _name = name; return *this; }

    Item* item() const { return _item; }
    ItemSource& item(Item* item) { _item = item; return *this; }

    uint8_t item_id() const { return (_item) ? _item->id() : ITEM_NONE; }

    WorldRegion* region() const { return _region; }
    ItemSource& region(WorldRegion* region) { _region = region; return *this; }

    const std::vector<std::string>& hints() const { return _hints; }
    void add_hint(const std::string& hint) { _hints.push_back(hint); }
    
    virtual bool is_item_compatible(Item* item) const = 0;
    virtual void write_to_rom(md::ROM& rom) const = 0;

    virtual Json to_json() const;
    static ItemSource* from_json(const Json& json, const std::map<std::string, WorldRegion*>& regions);  
};

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemSourceChest : public ItemSource
{
private:
    uint8_t _chest_id;
public:
    ItemSourceChest(uint8_t chest_id, const std::string& name, WorldRegion* region = nullptr, const std::vector<std::string>& hints = {}) :
        ItemSource (name, region, hints),
        _chest_id  (chest_id)
    {}

    virtual std::string type_name() const { return "chest"; }
    virtual bool is_item_compatible(Item* item) const { return true; }

    virtual void write_to_rom(md::ROM& rom) const
    {
        rom.setByte(0x9EABE + _chest_id, this->item_id());
    }

    virtual Json to_json() const;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemSourceOnGround : public ItemSource
{
private:
    std::vector<uint32_t> _addresses_in_rom;
    bool _cannot_be_taken_repeatedly;

public:
    ItemSourceOnGround(std::vector<uint32_t> addresses_in_rom, const std::string& name, WorldRegion* region, 
                        const std::vector<std::string>& hints, bool cannot_be_taken_repeatedly) :
        ItemSource                  (name, region, hints), 
        _addresses_in_rom           (addresses_in_rom),
        _cannot_be_taken_repeatedly (cannot_be_taken_repeatedly)
    {
        this->add_hint("lying on the ground, waiting for someone to pick it up");
    }

    virtual std::string type_name() const { return "ground"; }

    virtual bool is_item_compatible(Item* item) const
    {
        if(!item)
            return true;
        if(_cannot_be_taken_repeatedly)
            return true;
        return item->allowed_on_ground();
    }

    virtual void write_to_rom(md::ROM& rom) const
    {
        for (uint32_t address : _addresses_in_rom)
            rom.setByte(address, this->item_id() + 0xC0);
    }

    virtual Json to_json() const;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemSourceShop : public ItemSourceOnGround
{
public:
    ItemSourceShop(std::vector<uint32_t> addresses_in_rom, const std::string& name, WorldRegion* region, const std::vector<std::string>& hints) :
        ItemSourceOnGround (addresses_in_rom, name, region, hints, true)
    {
        this->add_hint("owned by someone trying to make profit out of it");
    }

    ItemSourceShop(uint32_t addressInROM, const std::string& name, WorldRegion* region = nullptr, const std::vector<std::string>& hints = {}) :
        ItemSourceShop(std::vector<uint32_t>({ addressInROM }), name, region, hints)
    {}

    virtual std::string type_name() const { return "shop"; }

    virtual bool is_item_compatible(Item* item) const;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemSourceReward : public ItemSource
{
private:
    uint32_t _address_in_rom;

public:
    ItemSourceReward(uint32_t address_in_rom, const std::string& name, WorldRegion* region, const std::vector<std::string>& hints) :
        ItemSource      (name, region, hints), 
        _address_in_rom (address_in_rom)
    {
        this->add_hint("owned by someone willing to give it to the brave");
    }

    virtual std::string type_name() const { return "reward"; }

    virtual bool is_item_compatible(Item* item) const
    {
        return (item->id() < ITEM_NONE);
    }

    virtual void write_to_rom(md::ROM& rom) const
    {
        rom.setByte(_address_in_rom, this->item_id());
    }

    virtual Json to_json() const;
};
