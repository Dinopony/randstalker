#pragma once

#include <string>
#include "item.hpp"

class WorldNode;

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemSource
{
private:
    std::string _name;
    Item* _item;
    WorldNode* _node;
    std::vector<std::string> _hints;

public:
    ItemSource(const std::string& name, WorldNode* node, const std::vector<std::string>& hints);

    virtual std::string type_name() const = 0;

    const std::string& name() const { return _name; }
    ItemSource& name(const std::string& name) { _name = name; return *this; }

    Item* item() const { return _item; }
    ItemSource& item(Item* item) { _item = item; return *this; }

    uint8_t item_id() const { return (_item) ? _item->id() : ITEM_NONE; }

    WorldNode* node() const { return _node; }
    ItemSource& node(WorldNode* node) { _node = node; return *this; }

    const std::vector<std::string>& hints() const { return _hints; }
    void add_hint(const std::string& hint) { _hints.push_back(hint); }
    
    virtual bool is_item_compatible(Item* item) const = 0;
    virtual void write_to_rom(md::ROM& rom) const = 0;

    virtual Json to_json() const;
    static ItemSource* from_json(const Json& json, const std::map<std::string, WorldNode*>& nodes);  
};

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemSourceChest : public ItemSource
{
private:
    uint8_t _chest_id;
public:
    ItemSourceChest(uint8_t chest_id, const std::string& name, WorldNode* node = nullptr, const std::vector<std::string>& hints = {}) :
        ItemSource (name, node, hints),
        _chest_id  (chest_id)
    {}

    virtual std::string type_name() const { return "chest"; }
    virtual bool is_item_compatible(Item* item) const { return true; }

    virtual void write_to_rom(md::ROM& rom) const
    {
        rom.set_byte(0x9EABE + _chest_id, this->item_id());
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
    ItemSourceOnGround(std::vector<uint32_t> addresses_in_rom, const std::string& name, WorldNode* node, 
                        const std::vector<std::string>& hints, bool cannot_be_taken_repeatedly) :
        ItemSource                  (name, node, hints), 
        _addresses_in_rom           (addresses_in_rom),
        _cannot_be_taken_repeatedly (cannot_be_taken_repeatedly)
    {
        if(this->type_name() == "ground")
        {
            this->add_hint("lying on the ground, waiting for someone to pick it up");
        }
    }

    virtual std::string type_name() const { return "ground"; }

    virtual bool is_item_compatible(Item* item) const
    {
        if(!item)
            return true;
        if(item->id() >= ITEM_GOLDS_START)
            return false;
        if(_cannot_be_taken_repeatedly)
            return true;
        return item->allowed_on_ground();
    }

    virtual void write_to_rom(md::ROM& rom) const
    {
        for (uint32_t address : _addresses_in_rom)
            rom.set_byte(address, this->item_id() + 0xC0);
    }

    virtual Json to_json() const;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemSourceShop : public ItemSourceOnGround
{
public:
    ItemSourceShop(std::vector<uint32_t> addresses_in_rom, const std::string& name, WorldNode* node, const std::vector<std::string>& hints) :
        ItemSourceOnGround (addresses_in_rom, name, node, hints, true)
    {
        this->add_hint("owned by someone trying to make profit out of it");
    }

    ItemSourceShop(uint32_t addressInROM, const std::string& name, WorldNode* node = nullptr, const std::vector<std::string>& hints = {}) :
        ItemSourceShop(std::vector<uint32_t>({ addressInROM }), name, node, hints)
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
    ItemSourceReward(uint32_t address_in_rom, const std::string& name, WorldNode* node, const std::vector<std::string>& hints) :
        ItemSource      (name, node, hints), 
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
        rom.set_byte(_address_in_rom, this->item_id());
    }

    virtual Json to_json() const;
};
