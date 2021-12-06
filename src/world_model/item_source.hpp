#pragma once

#include <string>
#include "item.hpp"
#include "entity.hpp"

#include "../constants/item_codes.hpp"

constexpr const char* ITEM_SOURCE_TYPE_CHEST = "chest";
constexpr const char* ITEM_SOURCE_TYPE_GROUND = "ground";
constexpr const char* ITEM_SOURCE_TYPE_SHOP = "shop";
constexpr const char* ITEM_SOURCE_TYPE_REWARD = "reward";

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemSource
{
private:
    std::string _name;
    Item* _item;
    std::string _node_id;
    std::vector<std::string> _hints;

public:
    ItemSource(const std::string& name, const std::string& node_id, const std::vector<std::string>& hints);
    virtual ~ItemSource() = default;

    virtual std::string type_name() const = 0;
    bool is_chest() { return this->type_name() == ITEM_SOURCE_TYPE_CHEST; }
    bool is_ground_item() { return this->type_name() == ITEM_SOURCE_TYPE_GROUND; }
    bool is_shop_item() { return this->type_name() == ITEM_SOURCE_TYPE_SHOP; }
    bool is_npc_reward() { return this->type_name() == ITEM_SOURCE_TYPE_REWARD; }

    const std::string& name() const { return _name; }
    void name(const std::string& name) { _name = name; }

    Item* item() const { return _item; }
    virtual void item(Item* item) { _item = item; }

    uint8_t item_id() const { return (_item) ? _item->id() : ITEM_NONE; }

    const std::string& node_id() const { return _node_id; }
    void node_id(const std::string& node_id) { _node_id = node_id; }

    const std::vector<std::string>& hints() const { return _hints; }
    void add_hint(const std::string& hint) { _hints.emplace_back(hint); }
    
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
    ItemSourceChest(uint8_t chest_id, const std::string& name, const std::string& node_id = "", const std::vector<std::string>& hints = {}) :
        ItemSource (name, node_id, hints),
        _chest_id  (chest_id)
    {}

    uint8_t chest_id() const { return _chest_id; }

    virtual std::string type_name() const { return ITEM_SOURCE_TYPE_CHEST; }
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
    ItemSourceOnGround(const std::string& name, std::vector<Entity*> entities, const std::string& node_id = "", 
                        const std::vector<std::string>& hints = {}, bool cannot_be_taken_repeatedly = false, bool add_hint = true) :
        ItemSource                  (name, node_id, hints), 
        _entities                   (entities),
        _cannot_be_taken_repeatedly (cannot_be_taken_repeatedly)
    {
        if(add_hint)
            this->add_hint("lying on the ground, waiting for someone to pick it up");
    }

    const std::vector<Entity*>& entities() const { return _entities; }

    virtual void item(Item* item)
    { 
        ItemSource::item(item);
        for (Entity* entity : _entities)
            entity->entity_type_id(this->item_id() + 0xC0);
    }

    virtual std::string type_name() const { return ITEM_SOURCE_TYPE_GROUND; }

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
    ItemSourceShop(const std::string& name, std::vector<Entity*> entities, const std::string& node_id = "", 
                    const std::vector<std::string>& hints = {}) :
        ItemSourceOnGround (name, entities, node_id, hints, true, false)
    {
        this->add_hint("owned by someone trying to make profit out of it");
    }

    virtual std::string type_name() const { return ITEM_SOURCE_TYPE_SHOP; }

    virtual bool is_item_compatible(Item* item) const;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemSourceReward : public ItemSource
{
private:
    uint32_t _address_in_rom;

public:
    ItemSourceReward(uint32_t address_in_rom, const std::string& name, const std::string& node_id = "", const std::vector<std::string>& hints = {}) :
        ItemSource      (name, node_id, hints), 
        _address_in_rom (address_in_rom)
    {
        this->add_hint("owned by someone willing to give it to the brave");
    }

    uint32_t address_in_rom() const { return _address_in_rom; }

    virtual std::string type_name() const { return ITEM_SOURCE_TYPE_REWARD; }

    virtual bool is_item_compatible(Item* item) const
    {
        return (item->id() < ITEM_NONE);
    }

    virtual Json to_json() const;
};
