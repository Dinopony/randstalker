#pragma once

#include "landstalker_lib/patches/game_patch.hpp"
#include "landstalker_lib/constants/entity_type_codes.hpp"

/**
 * When performing the tree-cutting glitch (feeding a goldbag to a tree to make it killable), they systematically
 * drop a Magic Sword. This patch does NOT fix the tree-cutting glitch, but removes the reward of doing so
 */
class PatchRemoveTreeCuttingGlitchDrops : public GamePatch
{
public:
    void inject_code(md::ROM& rom, World& world) override
    {
        constexpr uint32_t ADDR_DROP_ITEM_BOX = 0x162CC;
        constexpr uint32_t ADDR_DROP_NOTHING = 0x16270;

        md::Code proc_filter_sacred_tree_reward;
        proc_filter_sacred_tree_reward.cmpiw(0x126, addr_(reg_A5, 0xA));
        proc_filter_sacred_tree_reward.beq("not_a_tree");
        {
            proc_filter_sacred_tree_reward.movew((uint16_t)ENTITY_ITEM_BOX, addr_(reg_A5, 0x3A));
            proc_filter_sacred_tree_reward.jmp(ADDR_DROP_ITEM_BOX);
        }
        proc_filter_sacred_tree_reward.label("not_a_tree");
        proc_filter_sacred_tree_reward.jmp(ADDR_DROP_NOTHING);

        uint32_t proc_addr = rom.inject_code(proc_filter_sacred_tree_reward);
        rom.set_code(0x162C6, md::Code().jmp(proc_addr));
    }
};
