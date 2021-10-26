#pragma once

#include "../tools/megadrive/rom.hpp"
#include "../extlibs/json.hpp"
#include <sstream>

class Item
{
private:
    uint8_t     _id;
    std::string _name;
    uint8_t     _max_quantity;
    uint8_t     _starting_quantity;
    uint16_t    _gold_value;
    bool        _allowed_on_ground;

public:
    Item() {}

    Item(uint8_t id, const std::string& name, uint8_t max_quantity, uint16_t gold_value, bool allowed_on_ground = true) :
        _id                 (id),
        _name               (name),
        _max_quantity       (max_quantity),
        _starting_quantity  (0),
        _gold_value         (gold_value),
        _allowed_on_ground  (allowed_on_ground)
    {}
    
    uint8_t id() const { return _id; }
    Item& id(uint8_t id) { _id = id; return *this; }

    const std::string& name() const { return _name; }
    Item& name(const std::string& name) { _name = name; return *this; }

    uint8_t starting_quantity() const { return _starting_quantity; }
    Item& starting_quantity(uint8_t quantity) { _starting_quantity = quantity; return *this; }
    
    uint8_t max_quantity() const { return _max_quantity; }
    Item& max_quantity(uint8_t quantity) { _max_quantity = quantity; return *this; }

    uint16_t gold_value() const { return _gold_value; }
    virtual Item& gold_value(uint16_t value) { _gold_value = value; return *this; }

    bool allowed_on_ground() const { return _allowed_on_ground; }
    void allowed_on_ground(bool allowed) { _allowed_on_ground = allowed; }

    virtual void write_to_rom(md::ROM& rom) const;

    Json to_json() const;
    static Item* from_json(const Json& json);  
};

////////////////////////////////////////////////////////////////

class ItemGolds : public Item
{
public:
    ItemGolds(uint8_t id, uint16_t gold_value) : 
        Item(id, "", 0, 0, false)
    {
        this->gold_value(gold_value);
    }

    virtual Item& gold_value(uint16_t value)
    {
        Item::gold_value(value);

        std::ostringstream new_name;
        new_name << (uint32_t)value << " golds";
        name(new_name.str());

        return *this;
    }

    virtual void write_to_rom(md::ROM& rom) const
    {
        // Do nothing, since it is not a real item
        return;
    }
};

#define ITEM_EKEEKE 0x00
#define ITEM_MAGIC_SWORD 0x01
#define ITEM_ICE_SWORD 0x02
#define ITEM_THUNDER_SWORD 0x03
#define ITEM_GAIA_SWORD 0x04
#define ITEM_FIREPROOF_BOOTS 0x05
#define ITEM_IRON_BOOTS 0x06
#define ITEM_HEALING_BOOTS 0x07
#define ITEM_SPIKE_BOOTS 0x08
#define ITEM_STEEL_BREAST 0x09
#define ITEM_CHROME_BREAST 0x0A
#define ITEM_SHELL_BREAST 0x0B
#define ITEM_HYPER_BREAST 0x0C
#define ITEM_MARS_STONE 0x0D
#define ITEM_MOON_STONE 0x0E
#define ITEM_SATURN_STONE 0x0F
#define ITEM_VENUS_STONE 0x10
#define ITEM_AWAKENING_BOOK 0x11 // Fake item
#define ITEM_DETOX_GRASS 0x12
#define ITEM_GAIA_STATUE 0x13
#define ITEM_GOLDEN_STATUE 0x14
#define ITEM_MIND_REPAIR 0x15
#define ITEM_CASINO_TICKET 0x16
#define ITEM_AXE_MAGIC 0x17
#define ITEM_BLUE_RIBBON 0x18
#define ITEM_BUYER_CARD 0x19
#define ITEM_LANTERN 0x1A
#define ITEM_GARLIC 0x1B
#define ITEM_ANTI_PARALYZE 0x1C
#define ITEM_STATUE_JYPTA 0x1D
#define ITEM_SUN_STONE 0x1E
#define ITEM_ARMLET 0x1F
#define ITEM_EINSTEIN_WHISTLE 0x20
#define ITEM_BLUE_JEWEL 0x21 // Detox Book in base game
#define ITEM_YELLOW_JEWEL 0x22 // AntiCurse Book in base game
#define ITEM_RECORD_BOOK 0x23
#define ITEM_SPELL_BOOK 0x24
#define ITEM_HOTEL_REGISTER 0x25 // Fake item
#define ITEM_ISLAND_MAP 0x26 // Fake item
#define ITEM_LITHOGRAPH 0x27
#define ITEM_RED_JEWEL 0x28
#define ITEM_PAWN_TICKET 0x29
#define ITEM_PURPLE_JEWEL 0x2A
#define ITEM_GOLA_EYE 0x2B
#define ITEM_DEATH_STATUE 0x2C
#define ITEM_DAHL 0x2D
#define ITEM_RESTORATION 0x2E
#define ITEM_LOGS 0x2F
#define ITEM_ORACLE_STONE 0x30
#define ITEM_IDOL_STONE 0x31
#define ITEM_KEY 0x32
#define ITEM_SAFETY_PASS 0x33
#define ITEM_GREEN_JEWEL 0x34 // No52 in base game (unused item)
#define ITEM_BELL 0x35
#define ITEM_SHORT_CAKE 0x36
#define ITEM_GOLA_NAIL 0x37
#define ITEM_GOLA_HORN 0x38
#define ITEM_GOLA_FANG 0x39
#define ITEM_NO_SWORD 0x3A
#define ITEM_NO_ARMOR 0x3B
#define ITEM_NO_BOOTS 0x3C
#define ITEM_NO_RING 0x3D
#define ITEM_LIFESTOCK 0x3E
#define ITEM_NONE 0x3F
#define ITEM_GOLDS_START 0x40
