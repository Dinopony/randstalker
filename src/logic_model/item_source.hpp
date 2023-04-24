#pragma once

#include <string>
#include <utility>
#include <landstalker_lib/model/item.hpp>
#include <landstalker_lib/model/entity.hpp>
#include <landstalker_lib/constants/item_codes.hpp>

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

    [[nodiscard]] virtual std::string type_name() const = 0;
    [[nodiscard]] bool is_chest() const { return this->type_name() == ITEM_SOURCE_TYPE_CHEST; }
    [[nodiscard]] bool is_ground_item() const { return this->type_name() == ITEM_SOURCE_TYPE_GROUND; }
    [[nodiscard]] bool is_shop_item() const { return this->type_name() == ITEM_SOURCE_TYPE_SHOP; }
    [[nodiscard]] bool is_npc_reward() const { return this->type_name() == ITEM_SOURCE_TYPE_REWARD; }

    [[nodiscard]] const std::string& name() const { return _name; }
    void name(const std::string& name) { _name = name; }

    [[nodiscard]] Item* item() const { return _item; }
    virtual void item(Item* item) { _item = item; }

    /// A source is empty if it contains no item, but not if it contains ITEM_NONE
    [[nodiscard]] bool empty() const { return _item == nullptr; }

    [[nodiscard]] uint8_t item_id() const { return (_item) ? _item->id() : ITEM_NONE; }

    [[nodiscard]] const std::string& node_id() const { return _node_id; }
    void node_id(const std::string& node_id) { _node_id = node_id; }

    [[nodiscard]] const std::vector<std::string>& hints() const { return _hints; }
    void add_hint(const std::string& hint) { _hints.emplace_back(hint); }

    [[nodiscard]] virtual Json to_json() const;
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

    [[nodiscard]] uint8_t chest_id() const { return _chest_id; }
    [[nodiscard]] std::string type_name() const override { return ITEM_SOURCE_TYPE_CHEST; }
    [[nodiscard]] Json to_json() const override;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemSourceOnGround : public ItemSource
{
private:
    std::vector<Entity*> _entities;
    uint8_t _ground_item_id = 0x00;

public:
    ItemSourceOnGround(const std::string& name, std::vector<Entity*> entities, uint8_t ground_item_id, const std::string& node_id = "",
                        const std::vector<std::string>& hints = {}, bool add_hint = true) :
        ItemSource                  (name, node_id, hints), 
        _entities                   (std::move(entities)),
        _ground_item_id             (ground_item_id)
    {
        if(add_hint)
            this->add_hint("lying on the ground, waiting for someone to pick it up");
    }

    [[nodiscard]] const std::vector<Entity*>& entities() const { return _entities; }
    [[nodiscard]] uint8_t ground_item_id() const { return _ground_item_id; }
    [[nodiscard]] std::string type_name() const override { return ITEM_SOURCE_TYPE_GROUND; }
    [[nodiscard]] Json to_json() const override;
};

///////////////////////////////////////////////////////////////////////////////////////////////

class ItemSourceShop : public ItemSourceOnGround
{
public:
    ItemSourceShop(const std::string& name, std::vector<Entity*> entities, uint8_t shop_item_id, const std::string& node_id = "",
                    const std::vector<std::string>& hints = {}) :
        ItemSourceOnGround (name, std::move(entities), shop_item_id, node_id, hints, false)
    {
        this->add_hint("owned by someone trying to make profit out of it");
    }

    [[nodiscard]] std::string type_name() const override { return ITEM_SOURCE_TYPE_SHOP; }
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

    [[nodiscard]] uint32_t address_in_rom() const { return _address_in_rom; }
    [[nodiscard]] std::string type_name() const override { return ITEM_SOURCE_TYPE_REWARD; }
    [[nodiscard]] Json to_json() const override;
};
