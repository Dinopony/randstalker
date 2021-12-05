#include "entity_type.hpp"
#include "world.hpp"

EntityType* EntityType::from_json(uint8_t id, const Json& json, const World& world)
{
    const std::string& name = json.at("name");
    const std::string& type = json.at("type");

    if(type == "entity")
    {
        return new EntityType(id, name);
    }
    else if(type == "enemy")
    {
        uint8_t health = json.value("health", 0);
        uint8_t attack = json.value("attack", 0);
        uint8_t defence = json.value("defence", 99);
        uint8_t dropped_golds = json.value("droppedGolds", 0);
        
        std::string dropped_item_name = json.value("droppedItem", "");
        Item* dropped_item = nullptr;
        if(!dropped_item_name.empty())
            dropped_item = world.item(dropped_item_name);

        uint16_t drop_probability = json.value("dropProbability", 0);

        return new EntityEnemy(id, name, health, attack, defence, dropped_golds, dropped_item, drop_probability);
    }

    return nullptr;
}

EntityEnemy::EntityEnemy(uint8_t id, const std::string& name, uint8_t health, uint8_t attack, uint8_t defence, 
        uint8_t dropped_golds, Item* dropped_item, uint16_t drop_probability) :
    EntityType              (id, name),
    _health             (health),
    _attack             (attack),
    _defence            (defence),
    _dropped_golds      (dropped_golds),
    _dropped_item       (dropped_item),
    _drop_probability   (drop_probability),
    _unkillable         (false)
{}

void EntityEnemy::apply_health_factor(double factor)
{
    if(_health < 255)
    {
        double factored_health = static_cast<uint16_t>(_health) * factor;
        if (factored_health > 254)
            factored_health = 254;
        _health = static_cast<uint8_t>(factored_health);
    }
}

void EntityEnemy::apply_armor_factor(double factor)
{
    if(_defence < 99)
    {
        double factored_defence = static_cast<uint16_t>(_defence) * factor;
        if (factored_defence > 98)
            factored_defence = 98;
        _defence = static_cast<uint8_t>(factored_defence);
    }
}

void EntityEnemy::apply_damage_factor(double factor)
{
    if(_attack < 127)
    {
        double factored_attack = static_cast<uint16_t>(_attack) * factor;
        if (factored_attack > 126)
            factored_attack = 126;
        _attack = static_cast<uint8_t>(factored_attack);
    }
}

void EntityEnemy::apply_golds_factor(double factor)
{
    double factored_golds = static_cast<uint16_t>(_dropped_golds) * factor;
    if (factored_golds > 255)
        factored_golds = 255;
    _dropped_golds = static_cast<uint8_t>(factored_golds);
}

void EntityEnemy::apply_drop_chance_factor(double factor)
{
    if(_drop_probability > 0 && _dropped_item->id() != 1)
    {
        double factored_drop_rate = _drop_probability / factor;
        if (factored_drop_rate < 1)
            factored_drop_rate = 1;
        _drop_probability = static_cast<uint16_t>(factored_drop_rate);
    }
}

Json EntityEnemy::to_json() const
{
    Json json = EntityType::to_json();

    json["health"] = _health;
    json["attack"] = _attack;
    json["defence"] = _defence;
    if(_dropped_golds)
        json["droppedGolds"] = _dropped_golds;
    if(_drop_probability > 0)
    {
        json["droppedItem"] = _dropped_item->name();
        json["dropProbability"] = _drop_probability;
    }
    if(_unkillable)
        json["unkillable"] = _unkillable;

    return json;
}

void EntityEnemy::apply_json(const Json& json, const World& world)
{
    EntityType::apply_json(json, world);

    if(json.contains("health"))
        _health = json.at("health");
    if(json.contains("attack"))
        _attack = json.at("attack");
    if(json.contains("defence"))
        _defence = json.at("defence");
    if(json.contains("droppedGolds"))
        _dropped_golds = json.at("droppedGolds");
    if(json.contains("droppedItem"))
    {
        std::string dropped_item_name = json.at("droppedItem");
        _dropped_item = world.item(dropped_item_name);
    }
    if(json.contains("dropProbability"))
        _drop_probability = json.at("dropProbability");
    if(json.contains("unkillable"))
        _unkillable = json.at("unkillable");
}
