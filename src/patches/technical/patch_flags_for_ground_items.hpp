#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include "../../logic_model/randomizer_world.hpp"
#include "../../logic_model/item_source.hpp"
#include <landstalker-lib/model/entity.hpp>

/**
 * In the vanilla game engine, the game almost never sets a flag when obtaining a ground item / shop item for the
 * first time, making them basically infinite.
 * This patch adds a mechanism to set flags when obtaining such items for the first time, which can be used for
 * two main purposes:
 *      - tracking for Archipelago client
 *      - preventing the player from taking the same item several times
 */
class PatchFlagsForGroundItems : public GamePatch
{
private:
    static constexpr uint8_t OFFSET_ENTITY_GROUND_ITEM_ID = 0x3A;
    static constexpr uint32_t GROUND_ITEM_FLAGS_START_ADDR = 0xFF1060;
    static constexpr uint32_t SHOP_ITEM_FLAGS_START_ADDR = 0xFF1064;

    std::vector<uint8_t> _finite_ground_items;
    std::vector<uint8_t> _finite_shop_items;

public:
    explicit PatchFlagsForGroundItems(const RandomizerOptions& options) :
        _finite_ground_items(options.finite_ground_items()),
        _finite_shop_items(options.finite_shop_items())
    {
        // If armor upgrades are enabled, grabbing an armor can give another one, which would make it potentially
        // obtaineable several times from the same source.
        if(options.use_armor_upgrades())
        {
            _finite_ground_items.emplace_back(ITEM_STEEL_BREAST);
            _finite_ground_items.emplace_back(ITEM_CHROME_BREAST);
            _finite_ground_items.emplace_back(ITEM_SHELL_BREAST);
            _finite_ground_items.emplace_back(ITEM_HYPER_BREAST);

            _finite_shop_items.emplace_back(ITEM_STEEL_BREAST);
            _finite_shop_items.emplace_back(ITEM_CHROME_BREAST);
            _finite_shop_items.emplace_back(ITEM_SHELL_BREAST);
            _finite_shop_items.emplace_back(ITEM_HYPER_BREAST);
        }

        // If jewels are in "Kazalt Jewel" mode, Red Jewel must not be obtained several times from the same source
        if(options.jewel_count() > MAX_INDIVIDUAL_JEWELS)
        {
            _finite_ground_items.emplace_back(ITEM_RED_JEWEL);
            _finite_shop_items.emplace_back(ITEM_RED_JEWEL);
        }

        // Archipelago items have no point in being obtained several times
        if(options.archipelago_world())
        {
            _finite_ground_items.emplace_back(ITEM_ARCHIPELAGO);
            _finite_shop_items.emplace_back(ITEM_ARCHIPELAGO);
        }
    }

    void alter_world(World& world) override
    {
        RandomizerWorld& rando_world = reinterpret_cast<RandomizerWorld&>(world);

        set_dialogue_as_ground_item_id(rando_world);
        add_mask_flags_for_already_taken_items(rando_world);
        add_starting_flags_for_unobtainable_items(rando_world);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        handle_set_flag_for_item_entity(rom);
    }

private:
    /**
     * This patch introduces a new value, the "ground_item_id" which is a unique identifier for all ground/shop item
     * sources. This function takes the ground_item_id values for all those item sources, and then set the
     * formerly unused "dialogue" value to this ground_item_id for all related entities.
     * @param world
     */
    void set_dialogue_as_ground_item_id(RandomizerWorld& world)
    {
        for(ItemSource* item_source : world.item_sources())
        {
            if(item_source->is_ground_item() || item_source->is_shop_item())
            {
                ItemSourceOnGround* ground_source = reinterpret_cast<ItemSourceOnGround*>(item_source);
                for(Entity* entity : ground_source->entities())
                    entity->dialogue(ground_source->ground_item_id());
            }
        }
    }

    static Flag get_checked_flag_for_item_source(ItemSourceOnGround* item_source)
    {
        uint8_t ground_item_id = item_source->ground_item_id();
        uint8_t flag_byte = ground_item_id >> 3;
        uint8_t flag_bit = ground_item_id & 0x7;

        if(item_source->is_shop_item())
            return { (uint16_t)(SHOP_ITEM_FLAGS_START_ADDR + flag_byte), flag_bit };
        else
            return { (uint16_t)(GROUND_ITEM_FLAGS_START_ADDR + flag_byte), flag_bit };
    }

    /**
     * This function uses the flags referenced through "ground_item_id" to mask already taken items if settings
     * require this.
     * @param world
     */
    void add_mask_flags_for_already_taken_items(RandomizerWorld& world)
    {
        for(ItemSource* item_source : world.item_sources())
        {
            if(item_source->is_ground_item())
            {
                if(!vectools::contains(_finite_ground_items, item_source->item_id()))
                    continue;
            }
            else if(item_source->is_shop_item())
            {
                if(!vectools::contains(_finite_shop_items, item_source->item_id()))
                    continue;
            }
            else continue; // Not a ground/shop item_source

            // If we reach this point, it means the item source contains an item that is considered as finite
            // and must be removed if already taken by the player
            ItemSourceOnGround* ground_source = reinterpret_cast<ItemSourceOnGround*>(item_source);
            Flag flag_source_already_taken = get_checked_flag_for_item_source(ground_source);
            for(Entity* entity : ground_source->entities())
                entity->remove_when_flag_is_set(flag_source_already_taken);
        }
    }

    /**
     * This function adds all flags related to empty item sources as starting flags to mark those as taken.
     * This has no other use than giving the possibility to have all 100% check flags, granting the golden credits.
     */
    void add_starting_flags_for_unobtainable_items(RandomizerWorld& rando_world)
    {
        for(ItemSource* item_source : rando_world.item_sources())
        {
            if(item_source->item_id() != ITEM_NONE)
                continue;

            if(!item_source->is_ground_item() && !item_source->is_shop_item())
                continue;

            // If we reach this point, it means the item source will never be obtainable, so we need to add the
            // "checked" flag on game start
            Flag flag = get_checked_flag_for_item_source(reinterpret_cast<ItemSourceOnGround*>(item_source));
            rando_world.starting_flags().emplace_back(flag);
        }
    }

    /**
     * This function adds the required mechanisms in game code to set flags when ground items and shop items
     * are obtained for the first time.
     * @param rom
     */
    static void handle_set_flag_for_item_entity(md::ROM& rom)
    {
        md::Code func;
        func.movem_to_stack({ reg_D0, reg_D1 }, {});
        {
            func.clrl(reg_D0);
            func.moveb(addr_(reg_A5, OFFSET_ENTITY_GROUND_ITEM_ID), reg_D0);
            func.lsrb(2, reg_D0);                                               // D0 contains flag ID
            func.movel(reg_D0, reg_D1);
            func.andib(0x07, reg_D1);                                           // D1 contains flag bit
            func.lsrb(3, reg_D0);                                               // D0 contains flag byte
            func.bset(reg_D1, addr_(reg_A0, reg_D0));
        }
        func.movem_from_stack({ reg_D0, reg_D1 }, {});
        func.rts();
        uint32_t func_set_flag_addr = rom.inject_code(func);

        // Add a procedure extension to call the new set_flag function when grabbing a ground item
        md::Code ground_hook_func;
        {
            ground_hook_func.movew(reg_D2, reg_D0);
            ground_hook_func.cmpib(ITEM_ARCHIPELAGO, reg_D0);
            ground_hook_func.bne("not_archipelago");
            {
                ground_hook_func.jsr(0x291D6);  // GetItem
            }
            ground_hook_func.label("not_archipelago");

            ground_hook_func.movem_to_stack({}, { reg_A0 });
            ground_hook_func.lea(GROUND_ITEM_FLAGS_START_ADDR, reg_A0);
            ground_hook_func.jsr(func_set_flag_addr);
            ground_hook_func.movem_from_stack({}, { reg_A0 });
        }
        ground_hook_func.rts();
        uint32_t ground_hook_func_addr = rom.inject_code(ground_hook_func);
        rom.set_code(0x24B1A, md::Code().jsr(ground_hook_func_addr));  // Ground items

        // Add a procedure extension to call the new set_flag function when buying a shop item
        md::Code shop_hook_func;
        {
            shop_hook_func.movew(reg_D2, reg_D0);
            ground_hook_func.cmpib(ITEM_ARCHIPELAGO, reg_D0);
            ground_hook_func.bne("not_archipelago");
            {
                ground_hook_func.jsr(0x291D6);  // GetItem
            }
            ground_hook_func.label("not_archipelago");

            shop_hook_func.movem_to_stack({}, { reg_A0 });
            shop_hook_func.lea(SHOP_ITEM_FLAGS_START_ADDR, reg_A0);
            shop_hook_func.jsr(func_set_flag_addr);
            shop_hook_func.movem_from_stack({}, { reg_A0 });
        }
        shop_hook_func.rts();
        uint32_t shop_hook_func_addr = rom.inject_code(shop_hook_func);
        rom.set_code(0x24F3E, md::Code().jsr(shop_hook_func_addr));  // Shop items
    }
};
