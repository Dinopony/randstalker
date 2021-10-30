#include "enemy.hpp"
#include "../world.hpp"

Enemy::Enemy(uint8_t id, const std::string& name, uint8_t health, uint8_t attack, uint8_t defence, 
        uint8_t dropped_golds, Item* dropped_item, uint16_t drop_probability) :
    _id                 (id),
    _name               (name),
    _health             (health),
    _attack             (attack),
    _defence            (defence),
    _dropped_golds      (dropped_golds),
    _dropped_item       (dropped_item),
    _drop_probability   (drop_probability)
{}

Json Enemy::to_json() const
{
    Json json;

    json["name"] = _name;
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

    return json;
}

Enemy* Enemy::from_json(uint8_t id, const Json& json, const World& world)
{
    std::string name = json.at("name");
    uint8_t health = json.at("health");
    uint8_t attack = json.at("attack");
    uint8_t defence = json.at("defence");
    uint8_t dropped_golds = json.value("droppedGolds", 0);
    
    std::string dropped_item_name = json.value("droppedItem", "");
    Item* dropped_item = nullptr;
    if(!dropped_item_name.empty())
        dropped_item = world.item(dropped_item_name);

    uint16_t drop_probability = json.value("dropProbability", 0);

    return new Enemy(id, name, health, attack, defence, dropped_golds, dropped_item, drop_probability);
}

void Enemy::apply_json(const Json& json, const World& world)
{
    if(json.contains("name"))
        _name = json.at("name");
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
}