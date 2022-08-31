#pragma once

#include <landstalker_lib/patches/game_patch.hpp>
#include <landstalker_lib/constants/offsets.hpp>
#include <landstalker_lib/constants/item_codes.hpp>

#include "../../logic_model/hint_source.hpp"
#include "../../logic_model/randomizer_world.hpp"

class PatchSpellBookTeleportOnUse : public GamePatch
{
private:
    bool _consumable_spell_book = false;

public:
    explicit PatchSpellBookTeleportOnUse(bool consumable_spell_book) : _consumable_spell_book(consumable_spell_book) {}

    void inject_code(md::ROM& rom, World& world) override
    {
        // Replace "consume Spell Book" by a return indicating success and signaling there is a post-use effect
        // to handle.
        md::Code func_pre_use;
        if(_consumable_spell_book)
        {
            func_pre_use.moveb(reg_D0, addr_(0xFF1152));
            func_pre_use.jsr(0x8B98); // ConsumeItem
        }
        func_pre_use.jmp(offsets::PROC_ITEM_USE_RETURN_SUCCESS_HAS_POST_USE);
        uint32_t func_pre_use_addr = rom.inject_code(func_pre_use);
        world.item(ITEM_SPELL_BOOK)->pre_use_address(func_pre_use_addr);

        uint32_t func_warp_to_start = inject_func_warp_to_start(rom, world);
        world.item(ITEM_SPELL_BOOK)->post_use_address(func_warp_to_start);
    }

private:
    static uint32_t inject_func_warp_to_start(md::ROM& rom, const World& world)
    {
        uint16_t spawn_x = world.spawn_location().position_x();
        uint16_t spawn_y = world.spawn_location().position_y();
        uint16_t spawn_position = (spawn_x << 8) + spawn_y;
        uint16_t spawn_map_id = world.spawn_location().map_id();

        md::Code func;
        func.movem_to_stack({ reg_D0_D7 }, { reg_A0_A6 });
        {
            func.movew(spawn_position, addr_(0xFF5400));
            func.movew(0x0708, addr_(0xFF5402)); // Reset subtiles position
            func.trap(0, { 0x00, 0x4D });
            func.jsr(0x44C);      // sub_44C --> warp special effect?
            func.movew(spawn_map_id, reg_D0); // Set MapID to spawn map
            func.movew(0x0000, addr_(0xFF5412)); // Reset player height
            func.moveb(0x00, addr_(0xFF5422)); // Reset ground height
            func.moveb(0x00, addr_(0xFF5439)); // ^
            func.jsr(0x1586E);    // SetRoomNumber
            func.jsr(0x434);      // sub_434
            func.clrb(reg_D0);
            func.jsr(0x2824);     // LoadRoom_0
            func.jsr(0x410);      // sub_410
            func.jsr(0x8EB4);     // FadeInFromDarkness
        }
        func.movem_from_stack({ reg_D0_D7 }, { reg_A0_A6 });
        func.rts();
        return rom.inject_code(func);
    }
};
