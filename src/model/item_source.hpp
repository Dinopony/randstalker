#pragma once

#include <string>
#include "item.hpp"
#include "entity.hpp"

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
    void name(const std::string& name) { _name = name; }

    Item* item() const { return _item; }
    virtual void item(Item* item) { _item = item; }

    uint8_t item_id() const { return (_item) ? _item->id() : ITEM_NONE; }

    WorldNode* node() const { return _node; }
    void node(WorldNode* node) { _node = node; }

    const std::vector<std::string>& hints() const { return _hints; }
    void add_hint(const std::string& hint) { _hints.push_back(hint); }
    
    virtual bool is_item_compatible(Item* item) const = 0;

    virtual Json to_json() const;
    static ItemSource* from_json(const Json& json, const World& world);
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

    uint8_t chest_id() const { return _chest_id; }

    virtual std::string type_name() const { return "chest"; }
    virtual bool is_item_compatible(Item* item) const { return true; }

    virtual Json to_json() const;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemSourceOnGround : public ItemSource
{
private:
    std::vector<Entity*> _entities;
    bool _cannot_be_taken_repeatedly;

public:
    ItemSourceOnGround(const std::string& name, std::vector<Entity*> entities, WorldNode* node, 
                        const std::vector<std::string>& hints, bool cannot_be_taken_repeatedly) :
        ItemSource                  (name, node, hints), 
        _entities                   (entities),
        _cannot_be_taken_repeatedly (cannot_be_taken_repeatedly)
    {
        if(this->type_name() == "ground")
        {
            this->add_hint("lying on the ground, waiting for someone to pick it up");
        }
        
        for (Entity* entity : _entities)
            entity->entity_type_id(this->item_id() + 0xC0);
    }

    const std::vector<Entity*>& entities() const { return _entities; }

    virtual void item(Item* item)
    { 
        ItemSource::item(item);
        for (Entity* entity : _entities)
            entity->entity_type_id(this->item_id() + 0xC0);
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

    virtual Json to_json() const;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemSourceShop : public ItemSourceOnGround
{
public:
    ItemSourceShop(const std::string& name, std::vector<Entity*> entities, WorldNode* node, 
                    const std::vector<std::string>& hints) :
        ItemSourceOnGround (name, entities, node, hints, true)
    {
        this->add_hint("owned by someone trying to make profit out of it");
    }

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

    uint32_t address_in_rom() const { return _address_in_rom; }

    virtual std::string type_name() const { return "reward"; }

    virtual bool is_item_compatible(Item* item) const
    {
        return (item->id() < ITEM_NONE);
    }

    virtual Json to_json() const;
};
