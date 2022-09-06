#pragma once

#include "landstalker_lib/patches/game_patch.hpp"

#include "landstalker_lib/model/world.hpp"
#include "landstalker_lib/model/map.hpp"
#include "landstalker_lib/model/entity.hpp"
#include "landstalker_lib/constants/item_codes.hpp"
#include "landstalker_lib/constants/map_codes.hpp"
#include "landstalker_lib/constants/offsets.hpp"
#include "landstalker_lib/exceptions.hpp"
#include "landstalker_lib/tools/sprite.hpp"

#include "../../assets/blue_jewel.bin.hxx"
#include "../../assets/yellow_jewel.bin.hxx"
#include "../../assets/green_jewel.bin.hxx"

/**
 * The randomizer has an option to handle more jewels than the good old Red and Purple ones.
 * It can handle up to 9 jewels, but we only can afford having 5 unique jewel items.
 * This means there are two modes:
 *      - Unique jewels mode (1-5 jewels): each jewel has its own ID, name and sprite
 *      - Kazalt jewels mode (6+ jewels): jewels are one generic item that can be stacked up to 9 times
 *
 * This patch handles both modes, replacing useless items and injecting new sprites if needed.
 *
 * Green Jewel replaces No52 (an unused item)
 * Blue Jewel replaces Detox Book
 * Yellow Jewel replaces AntiCurse Book
 */
class PatchHandleJewels : public GamePatch
{
private:
    uint8_t _jewel_count = 2;

public:
    PatchHandleJewels(uint8_t jewel_count) : _jewel_count(jewel_count) {}

    void alter_world(World& world) override
    {
        if(_jewel_count > MAX_INDIVIDUAL_JEWELS)
        {
            Item* red_jewel = world.item(ITEM_RED_JEWEL);
            red_jewel->name("Kazalt Jewel");
            red_jewel->max_quantity(_jewel_count);
        }
        else
        {
            if(_jewel_count >= 5)
            {
                Item* item_yellow_jewel = new Item();
                item_yellow_jewel->id(ITEM_YELLOW_JEWEL);
                item_yellow_jewel->name("Yellow Jewel");
                item_yellow_jewel->gold_value(500);
                item_yellow_jewel->max_quantity(1);
                world.add_item(item_yellow_jewel);
            }
            if(_jewel_count >= 4)
            {
                Item* item_blue_jewel = new Item();
                item_blue_jewel->id(ITEM_BLUE_JEWEL);
                item_blue_jewel->name("Blue Jewel");
                item_blue_jewel->gold_value(500);
                item_blue_jewel->max_quantity(1);
                world.add_item(item_blue_jewel);
            }
            if(_jewel_count >= 3)
            {
                world.item(ITEM_GREEN_JEWEL)->name("Green Jewel");
                world.item(ITEM_GREEN_JEWEL)->gold_value(500);
                world.item(ITEM_GREEN_JEWEL)->max_quantity(1);
            }
        }

        // Remove jewels replaced from book IDs from priest stands if needed
        if(_jewel_count > 3 && _jewel_count <= MAX_INDIVIDUAL_JEWELS)
        {
            const std::vector<uint16_t> maps_to_clean = {
                    MAP_MASSAN_CHURCH, MAP_GUMI_CHURCH, MAP_RYUMA_CHURCH, MAP_MERCATOR_CHURCH_VARIANT, MAP_VERLA_CHURCH,
                    MAP_DESTEL_CHURCH, MAP_KAZALT_CHURCH
            };

            for(uint16_t map_id : maps_to_clean)
            {
                Map* map = world.map(map_id);
                for(int i = (int)map->entities().size()-1 ; i >= 0 ; --i)
                {
                    uint8_t type_id = map->entities()[i]->entity_type_id();
                    if(type_id == 0xC0 + ITEM_BLUE_JEWEL || type_id == 0xC0 + ITEM_YELLOW_JEWEL)
                        map->remove_entity(i);
                }
            }
        }
    }

    void alter_rom(md::ROM& rom) override
    {
        // Make the Awakening Book (the only one remaining in churches) heal all status conditions if other books
        // need to be removed to have item IDs for new jewels
        if(_jewel_count > 3 && _jewel_count <= MAX_INDIVIDUAL_JEWELS)
        {
            rom.set_code(0x24F6C, md::Code().nop(6));
            rom.set_code(0x24FB8, md::Code().moveb(0xFF, reg_D0));
        }

        // Change the behavior of AntiCurse and Detox books (now Yellow and Blue jewels) in shops
        rom.set_byte(0x24C40, 0x40);
        rom.set_byte(0x24C58, 0x40);
    }

    void inject_data(md::ROM& rom, World& world) override
    {
        // If we are in "Kazalt jewel" mode, don't do anything
        if(_jewel_count > MAX_INDIVIDUAL_JEWELS)
            return;

        SubSpriteMetadata subsprite(0x77FB);

        if(_jewel_count >= 3)
        {
            // Add a sprite for green jewel and make the item use it
            Sprite green_jewel_sprite(GREEN_JEWEL_SPRITE, GREEN_JEWEL_SPRITE_SIZE, { subsprite });
            uint32_t green_jewel_sprite_addr = rom.inject_bytes(green_jewel_sprite.encode());
            rom.set_long(offsets::ITEM_SPRITES_TABLE + (ITEM_GREEN_JEWEL * 0x4), green_jewel_sprite_addr);
        }
        if(_jewel_count >= 4)
        {
            // Add a sprite for blue jewel and make the item use it
            Sprite blue_jewel_sprite(BLUE_JEWEL_SPRITE, BLUE_JEWEL_SPRITE_SIZE, { subsprite });
            uint32_t blue_jewel_sprite_addr = rom.inject_bytes(blue_jewel_sprite.encode());
            rom.set_long(offsets::ITEM_SPRITES_TABLE + (ITEM_BLUE_JEWEL * 0x4), blue_jewel_sprite_addr);
        }
        if(_jewel_count >= 5)
        {
            // Add a sprite for green jewel and make the item use it
            Sprite yellow_jewel_sprite(YELLOW_JEWEL_SPRITE, YELLOW_JEWEL_SPRITE_SIZE, { subsprite });
            uint32_t yellow_jewel_sprite_addr = rom.inject_bytes(yellow_jewel_sprite.encode());
            rom.set_long(offsets::ITEM_SPRITES_TABLE + (ITEM_YELLOW_JEWEL * 0x4), yellow_jewel_sprite_addr);
        }
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        uint32_t func_reject_kazalt_tp = inject_func_reject_kazalt_tp(rom);
        add_jewel_check_for_kazalt_teleporter(rom, func_reject_kazalt_tp);
    }

private:
    static uint32_t inject_func_reject_kazalt_tp(md::ROM& rom)
    {
        md::Code func;
        {
            func.jsr(0x22EE8); // open textbox
            func.movew(0x22, reg_D0);
            func.jsr(0x28FD8); // display text
            func.jsr(0x22EA0); // close textbox
        }
        func.rts();
        return rom.inject_code(func);
    }

    void add_jewel_check_for_kazalt_teleporter(md::ROM& rom, uint32_t func_reject_kazalt_tp) const
    {
        md::Code proc_handle_jewels_check;

        if(_jewel_count > MAX_INDIVIDUAL_JEWELS)
        {
            proc_handle_jewels_check.movem_to_stack({reg_D1},{});
            proc_handle_jewels_check.moveb(addr_(0xFF1054), reg_D1);
            proc_handle_jewels_check.andib(0x0F, reg_D1);
            proc_handle_jewels_check.cmpib(_jewel_count, reg_D1); // Test if red jewel is owned
            proc_handle_jewels_check.movem_from_stack({reg_D1},{});
            proc_handle_jewels_check.ble("no_teleport");
        }
        else
        {
            if(_jewel_count >= 5)
            {
                proc_handle_jewels_check.btst(0x1, addr_(0xFF1051)); // Test if yellow jewel is owned
                proc_handle_jewels_check.beq("no_teleport");
            }
            if(_jewel_count >= 4)
            {
                proc_handle_jewels_check.btst(0x5, addr_(0xFF1050)); // Test if blue jewel is owned
                proc_handle_jewels_check.beq("no_teleport");
            }
            if(_jewel_count >= 3)
            {
                proc_handle_jewels_check.btst(0x1, addr_(0xFF105A)); // Test if green jewel is owned
                proc_handle_jewels_check.beq("no_teleport");
            }
            if(_jewel_count >= 2)
            {
                proc_handle_jewels_check.btst(0x1, addr_(0xFF1055)); // Test if purple jewel is owned
                proc_handle_jewels_check.beq("no_teleport");
            }
            if(_jewel_count >= 1)
            {
                proc_handle_jewels_check.btst(0x1, addr_(0xFF1054)); // Test if red jewel is owned
                proc_handle_jewels_check.beq("no_teleport");
            }
        }

        // Teleport to Kazalt
        proc_handle_jewels_check.moveq(0x7, reg_D0);
        proc_handle_jewels_check.jsr(0xE110);  // "func_teleport_kazalt"
        proc_handle_jewels_check.jmp(0x62FA);

        // Rejection message
        proc_handle_jewels_check.label("no_teleport");
        proc_handle_jewels_check.jsr(func_reject_kazalt_tp);
        proc_handle_jewels_check.rts();

        uint32_t handle_jewels_addr = rom.inject_code(proc_handle_jewels_check);

        // This adds the jewels as a requirement for the Kazalt teleporter to work correctly
        rom.set_code(0x62F4, md::Code().jmp(handle_jewels_addr));
    }
};
