#include <md_tools.hpp>

#include "../../constants/item_codes.hpp"
#include "../../tools/byte_array.hpp"
#include "../../world_model/item.hpp"
#include "../../world_model/world.hpp"

static uint32_t inject_gold_rewards_block(md::ROM& rom, World& world)
{
    ByteArray byte_array;

    for(uint32_t id=ITEM_GOLDS_START ; id < world.items().size() ; ++id)
    {
        Item* gold_item = world.items().at(id);
        byte_array.add_byte(static_cast<uint8_t>(gold_item->gold_value()));
    }

    return rom.inject_bytes(byte_array);
}

/**
 * In the original game, only 3 item IDs are reserved for gold rewards (3A, 3B, 3C)
 * Here, we moved the table of gold rewards to the end of the ROM so that we can handle 64 rewards up to 255 golds each.
 * In the new system, all item IDs after the "empty item" one (0x40 and above) are now gold rewards.
 */
void alter_gold_rewards_handling(md::ROM& rom, World& world)
{
    uint32_t gold_rewards_block = inject_gold_rewards_block(rom, world);

    rom.set_byte(0x0070DF, ITEM_GOLDS_START); // cmpi 3A, D0 >>> cmpi 40, D0
    rom.set_byte(0x0070E5, ITEM_GOLDS_START); // subi 3A, D0 >>> subi 40, D0

    // ------------- Function to put gold reward value in D0 ----------------
    // Input: D0 = gold reward ID (offset from 0x40)
    // Output: D0 = gold reward value

    md::Code func_get_gold_reward;

    func_get_gold_reward.movem_to_stack({}, { reg_A0 });
    func_get_gold_reward.lea(gold_rewards_block, reg_A0);
    func_get_gold_reward.moveb(addr_(reg_A0, reg_D0, md::Size::WORD), reg_D0);  // move.b (A0, D0.w), D0 : 1030 0000
    func_get_gold_reward.movem_from_stack({}, { reg_A0 });
    func_get_gold_reward.rts();

    uint32_t func_addr = rom.inject_code(func_get_gold_reward);

    // Set the call to the injected function
    // Before:      add D0,D0   ;   move.w (PC, D0, 42), D0
    // After:       jsr to injected function
    rom.set_code(0x0070E8, md::Code().jsr(func_addr));
}