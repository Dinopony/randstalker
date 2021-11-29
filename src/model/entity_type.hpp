#pragma once

#include "item.hpp"

class World;

class EntityType
{
private :
    uint8_t _id;
    std::string _name;

public:
    EntityType(uint8_t id, const std::string& name) :
        _id     (id),
        _name   (name)
    {}

    virtual std::string type_name() const { return "entity"; };

    uint8_t id() const { return _id; }
    std::string name() const { return _name; }

    virtual Json to_json() const
    {
        Json json;
        json["name"] = _name;
        json["type"] = this->type_name();
        return json;
    }

    static EntityType* from_json(uint8_t id, const Json& json, const World& world);

    virtual void apply_json(const Json& json, const World& world)
    {
        if(json.contains("name"))
            _name = json.at("name");
    }
};

class EntityEnemy : public EntityType
{
private :
    uint8_t _health;
    uint8_t _attack;
    uint8_t _defence;
    uint8_t _dropped_golds;
    Item* _dropped_item;
    uint16_t _drop_probability;  ///< 128 ---> 1/128 chance
    bool _unkillable;

public:
    EntityEnemy(uint8_t id, const std::string& name, uint8_t health, uint8_t attack, uint8_t defence, 
            uint8_t dropped_golds, Item* dropped_item, uint16_t drop_probability);

    virtual std::string type_name() const { return "enemy"; };

    uint8_t health() const { return _health; }
    void health(uint8_t health) { _health = health; }

    uint8_t attack() const { return _attack; }
    void attack(uint8_t attack) { _attack = attack; }

    uint8_t defence() const { return _defence; }
    void defence(uint8_t defence) { _defence = defence; }

    uint8_t dropped_golds() const { return (_unkillable) ? 0 : _dropped_golds; }
    void dropped_golds(uint8_t dropped_golds) { _dropped_golds = dropped_golds; }

    Item* dropped_item() const { return _dropped_item; }
    uint8_t dropped_item_id() const { return (_unkillable) ? 0x3F : _dropped_item->id(); }
    void dropped_item(Item* dropped_item) { _dropped_item = dropped_item; }

    uint16_t drop_probability() const { return (_unkillable) ? 1 : _drop_probability; }
    void drop_probability(uint16_t drop_probability) { _drop_probability = drop_probability; }

    bool unkillable() const { return _unkillable; }
    void unkillable(bool value) { _unkillable = value; }

    void apply_health_factor(double factor);
    void apply_armor_factor(double factor);
    void apply_damage_factor(double factor);
    void apply_golds_factor(double factor);
    void apply_drop_chance_factor(double factor);

    virtual Json to_json() const;
    virtual void apply_json(const Json& json, const World& world);
};

class EntityItemOnGround : public EntityType
{
private :
    Item* _item;

public:
    EntityItemOnGround(uint8_t id, const std::string& name, Item* item) :
        EntityType  (id, name),
        _item   (item)
    {}

    virtual std::string type_name() const { return "ground_item"; };

    Item* item() const { return _item; }

    virtual Json to_json() const
    {
        Json json = EntityType::to_json();
        json["itemId"] = _item->id();
        return json;
    }
};
