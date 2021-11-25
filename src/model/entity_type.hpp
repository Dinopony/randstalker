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


constexpr uint8_t ENEMY_SLIME_1 = 0x3C;
constexpr uint8_t ENEMY_SLIME_2 = 0x3D;
constexpr uint8_t ENEMY_SLIME_3 = 0x3E;
constexpr uint8_t ENEMY_SLIME_4 = 0x81;
constexpr uint8_t ENEMY_SLIME_5 = 0x82;
constexpr uint8_t ENEMY_SLIME_6 = 0x83;
constexpr uint8_t ENEMY_ORC_1 = 0x04;
constexpr uint8_t ENEMY_ORC_2 = 0x05;
constexpr uint8_t ENEMY_ORC_3 = 0x06;
constexpr uint8_t ENEMY_WORM_1 = 0x07;
constexpr uint8_t ENEMY_WORM_2 = 0x08;
constexpr uint8_t ENEMY_WORM_3 = 0x09;
constexpr uint8_t ENEMY_MUSHROOM_1 = 0x36;
constexpr uint8_t ENEMY_MUSHROOM_2 = 0x37;
constexpr uint8_t ENEMY_MUSHROOM_3 = 0x38;
constexpr uint8_t ENEMY_GIANT_1 = 0x39;
constexpr uint8_t ENEMY_GIANT_2 = 0x3A;
constexpr uint8_t ENEMY_GIANT_3 = 0x3B;
constexpr uint8_t ENEMY_REAPER_1 = 0x88;
constexpr uint8_t ENEMY_REAPER_2 = 0x89;
constexpr uint8_t ENEMY_GOLEM_1 = 0x92;
constexpr uint8_t ENEMY_GOLEM_2 = 0x93;
constexpr uint8_t ENEMY_GOLEM_3 = 0x94;
constexpr uint8_t ENEMY_SPECTRE_1 = 0x95;
constexpr uint8_t ENEMY_SPECTRE_2 = 0x96;
constexpr uint8_t ENEMY_SPECTRE_3 = 0x97;
constexpr uint8_t ENEMY_NINJA_1 = 0x17;
constexpr uint8_t ENEMY_NINJA_2 = 0x18;
constexpr uint8_t ENEMY_NINJA_3 = 0x19;
constexpr uint8_t ENEMY_LIZARD_1 = 0x1A;
constexpr uint8_t ENEMY_LIZARD_2 = 0x1B;
constexpr uint8_t ENEMY_LIZARD_3 = 0x1C;
constexpr uint8_t ENEMY_KNIGHT_1 = 0x1D;
constexpr uint8_t ENEMY_KNIGHT_2 = 0x1E;
constexpr uint8_t ENEMY_KNIGHT_3 = 0x1F;
constexpr uint8_t ENEMY_GHOST_1 = 0x20;
constexpr uint8_t ENEMY_GHOST_2 = 0x21;
constexpr uint8_t ENEMY_GHOST_3 = 0x22;
constexpr uint8_t ENEMY_MUMMY_1 = 0x23;
constexpr uint8_t ENEMY_MUMMY_2 = 0x24;
constexpr uint8_t ENEMY_MUMMY_3 = 0x25;
constexpr uint8_t ENEMY_UNICORN_1 = 0x26;
constexpr uint8_t ENEMY_UNICORN_2 = 0x27;
constexpr uint8_t ENEMY_UNICORN_3 = 0x28;
constexpr uint8_t ENEMY_SKELETON_1 = 0x29;
constexpr uint8_t ENEMY_SKELETON_2 = 0x2A;
constexpr uint8_t ENEMY_SKELETON_3 = 0x2B;
constexpr uint8_t ENEMY_MIMIC_1 = 0x2C;
constexpr uint8_t ENEMY_MIMIC_2 = 0x2D;
constexpr uint8_t ENEMY_MIMIC_3 = 0x2E;
constexpr uint8_t ENEMY_SPINNER_1 = 0x9D;
constexpr uint8_t ENEMY_SPINNER_2 = 0xAA;
constexpr uint8_t ENEMY_QUAKE_1 = 0xA7;
constexpr uint8_t ENEMY_QUAKE_2 = 0xAB;
constexpr uint8_t ENEMY_DUKE = 0x70;
constexpr uint8_t ENEMY_MIR = 0x7C;
constexpr uint8_t ENEMY_ZAK = 0x85;
constexpr uint8_t ENEMY_MIRO_1 = 0x00;
constexpr uint8_t ENEMY_MIRO_2 = 0x9F;
constexpr uint8_t ENEMY_IFRIT = 0xA0;
constexpr uint8_t ENEMY_NOLE = 0xA5;
constexpr uint8_t ENEMY_GOLA = 0xA2;
constexpr uint8_t ENEMY_SACRED_TREE = 75;

constexpr uint8_t ENTITY_CHEST = 18;
constexpr uint8_t ENTITY_SMALL_YELLOW_PLATFORM = 21;
constexpr uint8_t ENTITY_CRATE = 22;
constexpr uint8_t ENTITY_SMALL_GRAY_SPIKEBALL = 73;
constexpr uint8_t ENTITY_BUTTON = 85;
constexpr uint8_t ENTITY_SMALL_GREEN_PLATFORM = 88;
constexpr uint8_t ENTITY_SMALL_THIN_YELLOW_PLATFORM = 89;
constexpr uint8_t ENTITY_BIG_THIN_GREEN_PLATFORM = 94;
constexpr uint8_t ENTITY_BIG_THIN_YELLOW_PLATFORM = 95;
constexpr uint8_t ENTITY_BIG_GRAY_SPIKEBALL = 142;
constexpr uint8_t ENTITY_BIG_BLUE_BALL = 154;

constexpr uint8_t ENTITY_SOLDIER = 50;
constexpr uint8_t ENTITY_INJURED_DOG = 77;
constexpr uint8_t ENTITY_DEAD_BODY = 78;
constexpr uint8_t ENTITY_BOULDER = 98;
