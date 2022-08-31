#pragma once

#include "landstalker_lib/patches/game_patch.hpp"
#include "landstalker_lib/model/item.hpp"
#include "landstalker_lib/constants/item_codes.hpp"

class PatchArmorUpgrades : public GamePatch
{
public:
    void inject_code(md::ROM& rom, World& world) override
    {
        uint32_t function_alter_item_in_d0 = inject_function_alter_item_in_d0(rom);

        uint32_t function_change_item_in_reward_textbox = inject_function_change_item_in_reward_textbox(
                rom, function_alter_item_in_d0);

        uint32_t function_alter_item_given_by_ground_source = inject_function_alter_item_given_by_ground_source(
                rom, function_alter_item_in_d0);

        uint32_t function_alter_visible_item_for_ground_source = inject_function_alter_visible_item_for_ground_source(
                rom, function_change_item_in_reward_textbox);

        // Insert the actual calls to the previously injected functions

        // In 'chest reward' function, replace the item ID move by the injected function
        rom.set_code(0x0070BE, md::Code().jsr(function_change_item_in_reward_textbox));

        // In 'NPC reward' function, replace the item ID move by the injected function
        rom.set_code(0x028DD8, md::Code().jsr(function_change_item_in_reward_textbox));

        // In 'item on ground reward' function, replace the item ID move by the injected function
        rom.set_word(0x024ADC, 0x3002); // put the move D2,D0 before the jsr because it helps us while changing nothing to the usual logic
        rom.set_code(0x024ADE, md::Code().jsr(function_change_item_in_reward_textbox));

        // Replace 2928C lea (41F9 00FF1040) by a jsr to injected function
        rom.set_code(0x02928C, md::Code().jsr(function_alter_item_given_by_ground_source));

        // Replace 1963C - 19644 (0400 00C0 ; 1340 0036) by a jsr to a replacement function
        rom.set_code(0x01963C, md::Code().jsr(function_alter_visible_item_for_ground_source).nop());
    }

private:
    [[nodiscard]] static uint32_t inject_function_alter_item_in_d0(md::ROM& rom)
    {
        md::Code func;

        // Check if item ID is between 09 and 0C (armors). If not, branch to return.
        func.cmpib(ITEM_STEEL_BREAST, reg_D0);
        func.blt("return");
        func.cmpib(ITEM_HYPER_BREAST, reg_D0);
        func.bgt("return");
        {
            // By default, put Hyper breast as given armor
            func.movew(ITEM_HYPER_BREAST, reg_D0);

            func.btst(0x05, addr_(0xFF1045));
            func.bne("tier_2");
            {
                // Shell breast is not owned, put Shell breast
                func.movew(ITEM_SHELL_BREAST, reg_D0);
            }
            func.label("tier_2");
            func.btst(0x01, addr_(0xFF1045));
            func.bne("tier_1");
            {
                // Chrome breast is not owned, put Chrome breast
                func.movew(ITEM_CHROME_BREAST, reg_D0);
            }
            func.label("tier_1");
            func.btst(0x05, addr_(0xFF1044));
            func.bne("return");
            {
                // Steel breast is not owned, put Steel breast
                func.movew(ITEM_STEEL_BREAST, reg_D0);
            }
        }
        func.label("return");
        func.rts();

        return rom.inject_code(func);
    }

    [[nodiscard]] static uint32_t inject_function_change_item_in_reward_textbox(md::ROM& rom, uint32_t function_alter_item_in_d0)
    {
        md::Code func;
        {
            func.jsr(function_alter_item_in_d0);
            func.movew(reg_D0, addr_(0xFF1196));
            func.rts();
        }
        return rom.inject_code(func);
    }

    [[nodiscard]] static uint32_t inject_function_alter_item_given_by_ground_source(md::ROM& rom,
                                                                                    uint32_t function_alter_item_in_d0)
    {
        md::Code func;
        func.movem_to_stack({ reg_D7 }, { reg_A0 });
        {
            func.cmpib(ITEM_HYPER_BREAST, reg_D0);
            func.bgt("return");
            func.cmpib(ITEM_STEEL_BREAST, reg_D0);
            func.blt("return");
            {
                func.jsr(function_alter_item_in_d0);
                func.moveb(addr_(reg_A5, 0x3B), reg_D7);
                func.subib(0xC9, reg_D7);
                func.cmpa(lval_(0xFF5400), reg_A5); // ???
                func.blt("return");
                {
                    // set a flag when an armor is taken on the ground for it to disappear afterwards
                    func.bset(reg_D7, addr_(0xFF103F));
                }
            }
        }
        func.label("return");
        func.movem_from_stack({ reg_D7 }, { reg_A0 });
        func.lea(0xFF1040, reg_A0);
        func.rts();

        return rom.inject_code(func);
    }

    [[nodiscard]] static uint32_t inject_function_alter_visible_item_for_ground_source(md::ROM& rom,
                                                                                       uint32_t function_change_item_in_reward_textbox)
    {
        md::Code func;
        func.movem_to_stack({ reg_D7 }, { reg_A0 });  // movem D7,A0 -(A7)
        {
            func.subib(0xC0, reg_D0);
            func.cmpib(ITEM_HYPER_BREAST, reg_D0);
            func.bgt("place_item");
            func.cmpib(ITEM_STEEL_BREAST, reg_D0);
            func.blt("place_item");
            {
                func.moveb(reg_D0, reg_D7);
                func.subib(ITEM_STEEL_BREAST, reg_D7);
                func.btst(reg_D7, addr_(0xFF103F));
                func.bne("already_taken");
                {
                    // Item was NOT already taken, alter the armor inside
                    func.jsr(function_change_item_in_reward_textbox);
                    func.bra("place_item");
                }
                // Item was already taken, remove it by filling it with an empty item
                func.label("already_taken");
                func.movew(ITEM_NONE, reg_D0);
            }
            func.label("place_item");
            func.moveb(reg_D0, addr_(reg_A1, 0x36)); // move D0, ($36,A1) (1340 0036)
        }
        func.movem_from_stack({ reg_D7 }, { reg_A0 }); // movem (A7)+, D7,A0	(4CDF 0180)
        func.rts();

        return rom.inject_code(func);
    }
};
