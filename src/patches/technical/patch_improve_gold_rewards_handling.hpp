#pragma once

#include "landstalker_lib/patches/game_patch.hpp"

#include "landstalker_lib/constants/item_codes.hpp"
#include "landstalker_lib/tools/byte_array.hpp"
#include "landstalker_lib/model/item.hpp"

/**
 * In the original game, only 3 item IDs are reserved for fixed gold rewards (3A, 3B, 3C)
 * Here, we use a separate table which enables associating a 8-bit gold value to every invalid item ID (from 40 to FF).
 */
class PatchImproveGoldRewardsHandling : public GamePatch
{
private:
    uint32_t _gold_rewards_table_addr = 0xFFFFFFFF;

public:
    void inject_data(md::ROM& rom, World& world) override
    {
        ByteArray byte_array;
        for(size_t id=ITEM_GOLDS_START ; id < ITEM_GOLDS_END ; ++id)
        {
            try {
                Item* gold_item = world.item(id);
                byte_array.add_byte(static_cast<uint8_t>(gold_item->gold_value()));
            } catch(std::out_of_range&) {
                break;
            }
        }

        _gold_rewards_table_addr = rom.inject_bytes(byte_array);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        uint32_t function_check_gold_reward = inject_function_check_gold_reward(rom);
        handle_gold_rewards_for_chest(rom, function_check_gold_reward);
        handle_gold_rewards_for_npc(rom, function_check_gold_reward);
    }

private:
    /**
     * Read the gold value for the specified item, and puts it in a specific register.
     * Input: D0 = gold reward ID (offset from 0x40)
     * Output: D0 = gold reward value
     * @param rom
     * @return
     */
    uint32_t inject_function_check_gold_reward(md::ROM& rom) const
    {
        md::Code func_get_gold_reward;
        func_get_gold_reward.movem_to_stack({}, { reg_A0 });
        {
            func_get_gold_reward.lea(_gold_rewards_table_addr, reg_A0);
            func_get_gold_reward.moveb(addrw_(reg_A0, reg_D0), reg_D0);  // move.b (A0, D0.w), D0 : 1030 0000
        }
        func_get_gold_reward.movem_from_stack({}, { reg_A0 });
        func_get_gold_reward.rts();

        return rom.inject_code(func_get_gold_reward);
    }

    static void handle_gold_rewards_for_chest(md::ROM& rom, uint32_t function_check_gold_reward)
    {
        rom.set_byte(0x0070DF, ITEM_GOLDS_START); // cmpi 3A, D0 >>> cmpi 40, D0
        rom.set_byte(0x0070E5, ITEM_GOLDS_START); // subi 3A, D0 >>> subi 40, D0

        // Set the call to the injected function
        // Before:      add D0,D0   ;   move.w (PC, D0, 42), D0
        // After:       jsr to injected function
        rom.set_code(0x0070E8, md::Code().jsr(function_check_gold_reward));
    }

    static void handle_gold_rewards_for_npc(md::ROM& rom, uint32_t function_check_gold_reward)
    {
        constexpr uint32_t JUMP_ADDR_ITEM_REWARD = 0x28ECA;
        constexpr uint32_t JUMP_ADDR_GOLD_REWARD = 0x28EF0;
        constexpr uint32_t JUMP_ADDR_NO_REWARD = 0x28ED2;

        md::Code proc_check_if_item_is_golds;
        {
            proc_check_if_item_is_golds.clrl(reg_D0);
            proc_check_if_item_is_golds.movew(addr_(0xFF1196), reg_D0);
            proc_check_if_item_is_golds.cmpiw(ITEM_ARCHIPELAGO, reg_D0);
            proc_check_if_item_is_golds.bne("item_not_archipelago");
            {
                // Archipelago item case
                proc_check_if_item_is_golds.movew(0x1E, reg_D0);
                proc_check_if_item_is_golds.jmp(JUMP_ADDR_NO_REWARD);
            }
            proc_check_if_item_is_golds.label("item_not_archipelago");
            proc_check_if_item_is_golds.cmpiw(ITEM_GOLDS_START, reg_D0);
            proc_check_if_item_is_golds.blt("item_not_golds");
            {
                // Gold reward case
                proc_check_if_item_is_golds.subiw(ITEM_GOLDS_START, reg_D0);
                proc_check_if_item_is_golds.jsr(function_check_gold_reward);
                proc_check_if_item_is_golds.movel(reg_D0, addr_(0xFF1878));
                proc_check_if_item_is_golds.jmp(JUMP_ADDR_GOLD_REWARD);
            }
            proc_check_if_item_is_golds.label("item_not_golds");
            proc_check_if_item_is_golds.cmpiw(ITEM_NONE, reg_D0);
            proc_check_if_item_is_golds.bne("item_not_empty");
            {
                // No item case
                proc_check_if_item_is_golds.movew(0x1D, reg_D0);
                proc_check_if_item_is_golds.jmp(JUMP_ADDR_NO_REWARD);
            }
            proc_check_if_item_is_golds.label("item_not_empty");
            // Item reward case
            proc_check_if_item_is_golds.jmp(JUMP_ADDR_ITEM_REWARD);
        }
        uint32_t addr = rom.inject_code(proc_check_if_item_is_golds);

        rom.set_code(0x28EC4, md::Code().jmp(addr));
    }
};
