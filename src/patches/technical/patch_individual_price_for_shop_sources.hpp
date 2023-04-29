#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include "../../logic_model/randomizer_world.hpp"
#include "../../logic_model/item_source.hpp"

/**
 * In vanilla game, item prices in shops are determined by two factors:
 *  - a base price, set for each item in the game
 *  - a multiplier, set for each shop in the game
 * This means a given item can only have one price across the game, only slightly modified by the shop price multiplier
 * where it's put.
 *
 * This patch changes this behavior by fetching prices from a table where each shop item in the game has a fixed custom
 * price, determined at generation time.
 * This implies that an EkeEke can cost 10g at Ryuma while costing 1000g at Mercator
 */
class PatchIndividualPriceForShopSources : public GamePatch
{
private:
    uint32_t _gold_prices_table_addr = 0xFFFFFFFF;

public:
    void inject_data(md::ROM& rom, World& world) override
    {
        RandomizerWorld& rando_world = reinterpret_cast<RandomizerWorld&>(world);

        std::vector<uint16_t> prices_vector;
        for(ItemSource* source : rando_world.item_sources())
        {
            if(!source->is_shop_item())
                continue;

            ItemSourceShop* shop_source = reinterpret_cast<ItemSourceShop*>(source);

            uint8_t shop_item_id = shop_source->ground_item_id();
            if(prices_vector.size() <= shop_item_id)
                prices_vector.resize(shop_item_id+1, 0xFFFF);

            prices_vector[shop_item_id] = shop_source->price();
        }

        ByteArray prices_array;
        for(uint16_t price : prices_vector)
            prices_array.add_word(price);

        _gold_prices_table_addr = rom.inject_bytes(prices_array);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        md::Code func;
        {
            func.clrl(reg_D0);
            // Put shop item source ID in D0
            func.moveb(addr_(reg_A5, 0x3A), reg_D0);
            func.lsrw(1, reg_D0);
            // Put price in D1 and 0xFF1878
            func.lea(_gold_prices_table_addr, reg_A0);
            func.clrl(reg_D1);
            func.movew(addr_(reg_A0, reg_D0), reg_D1);
            func.movel(reg_D1, addr_(0xFF1878));
            // Put item ID in D0 and 0xFF1198
            func.clrl(reg_D0);
            func.moveb(addr_(0xFF1903), reg_D0);
            func.cmpib(ITEM_ARCHIPELAGO, reg_D0);
            func.bne("not_archipelago");
            {
                func.moveb(addr_(reg_A5, 0x3A), reg_D0);
                func.lsrb(2, reg_D0);
                func.addiw(ItemSourceShop::base_shop_uuid(), reg_D0);
                func.movew(reg_D0, addr_(PatchHandleArchipelago::ADDR_CURRENT_LOCATION_UUID));
                func.movew(ITEM_ARCHIPELAGO, reg_D0);
            }
            func.label("not_archipelago");
            func.movew(reg_D0, addr_(0xFF1198));
        }
        func.rts();

        uint32_t func_addr = rom.inject_code(func);
        rom.set_code(0x24CC2, md::Code().jmp(func_addr)); // Replace GetItemShopSellPrice by our own function
    }
};