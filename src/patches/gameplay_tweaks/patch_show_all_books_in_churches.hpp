#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <landstalker-lib/constants/map_codes.hpp>

/**
 * Usually, the game has a mechanism that makes items on ground disappear if the maximum quantity is owned
 * inside the inventory. This is also true for books in churches, and most notably the Record Book: if the player
 * has 9 consumable record books inside their inventory, they won't see the free Record Book inside churches
 * and won't be able to save until they use one of their Record Book in inventory.
 *
 * One map in the game circumvent this check: King Nole's infinite staircase, to always show the Gola items floating
 * in the air while opening the door.
 *
 * This patch extends the behavior of this map to all churches.
 */
class PatchShowAllBooksInChurches : public GamePatch
{
public:
    void inject_code(md::ROM& rom, World& world) override
    {
        std::vector<uint16_t> maps_to_check = {
                MAP_STAIRWAY_TO_NOLE,
                MAP_MASSAN_CHURCH, MAP_GUMI_CHURCH, MAP_RYUMA_CHURCH, MAP_MERCATOR_CHURCH,
                MAP_VERLA_CHURCH, MAP_DESTEL_CHURCH, MAP_KAZALT_CHURCH,
                MAP_LAKE_SHRINE_SAVE_ROOM, MAP_KN_LABYRINTH_SAVE_ROOM
        };

        constexpr uint32_t JUMP_ADDR_SKIP_ITEM_REMOVAL = 0x196BC;
        constexpr uint32_t JUMP_ADDR_DO_ITEM_REMOVAL = 0x1969A;

        md::Code func;
        func.movew(addr_(0xFF1204), reg_D2);
        for(uint16_t map_id : maps_to_check)
        {
            func.cmpiw(map_id, reg_D2);
            func.beq("special_map");
        }
        func.clrw(reg_D2);
        func.jmp(JUMP_ADDR_DO_ITEM_REMOVAL); // Regular map, do the item removal as usual

        func.label("special_map");
        func.clrw(reg_D2);
        func.jmp(JUMP_ADDR_SKIP_ITEM_REMOVAL); // Special map, do not perform the item removal

        uint32_t func_addr = rom.inject_code(func);
        rom.set_code(0x1968E, md::Code().jmp(func_addr).nop(3));
    }
};
