#include <landstalker_lib/md_tools.hpp>
#include <landstalker_lib/constants/offsets.hpp>
#include <landstalker_lib/constants/item_codes.hpp>
#include <landstalker_lib/model/world.hpp>
#include <landstalker_lib/model/item.hpp>

void handle_spell_book_teleport(md::ROM& rom, World& world, bool consumable_spell_book)
{
    uint16_t spawn_x = world.spawn_location().position_x();
    uint16_t spawn_y = world.spawn_location().position_y();
    uint16_t spawn_position = (spawn_x << 8) + spawn_y;
    uint16_t spawn_map_id = world.spawn_location().map_id();

    md::Code func_warp_to_start;
    func_warp_to_start.movem_to_stack({ reg_D0_D7 }, { reg_A0_A6 });
    func_warp_to_start.movew(spawn_position, addr_(0xFF5400));
    func_warp_to_start.movew(0x0708, addr_(0xFF5402)); // Reset subtiles position
    func_warp_to_start.trap(0, { 0x00, 0x4D });
    func_warp_to_start.jsr(0x44C);
    func_warp_to_start.movew(spawn_map_id, reg_D0); // Set MapID to spawn map
    func_warp_to_start.movew(0x0000, addr_(0xFF5412)); // Reset player height
    func_warp_to_start.moveb(0x00, addr_(0xFF5422)); // Reset ground height
    func_warp_to_start.moveb(0x00, addr_(0xFF5439)); // ^
    func_warp_to_start.jsr(0x1586E);
    func_warp_to_start.jsr(0x434);
    func_warp_to_start.clrb(reg_D0);
    func_warp_to_start.jsr(0x2824);
    func_warp_to_start.jsr(0x410);
    func_warp_to_start.jsr(0x8EB4);
    func_warp_to_start.movem_from_stack({ reg_D0_D7 }, { reg_A0_A6 });
    func_warp_to_start.rts();
    uint32_t func_warp_to_start_addr = rom.inject_code(func_warp_to_start);

    world.item(ITEM_SPELL_BOOK)->post_use_address(func_warp_to_start_addr);

    // Replace "consume Spell Book" by a return indicating success and signaling there is a post-use effect
    // to handle.
    md::Code func_pre_use;
    if(consumable_spell_book)
    {
        func_pre_use.moveb(reg_D0, addr_(0xFF1152));
        func_pre_use.jsr(0x8B98); // ConsumeItem
    }
    func_pre_use.jmp(offsets::PROC_ITEM_USE_RETURN_SUCCESS_HAS_POST_USE);
    uint32_t func_pre_use_addr = rom.inject_code(func_pre_use);

    world.item(ITEM_SPELL_BOOK)->pre_use_address(func_pre_use_addr);
}
