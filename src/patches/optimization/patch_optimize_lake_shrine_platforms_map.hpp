#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <landstalker-lib/model/map.hpp>
#include <landstalker-lib/model/entity.hpp>
#include <landstalker-lib/constants/map_codes.hpp>

/**
 * This patch optimizes a laggy map in Lake Shrine (the one with lots of falling platforms leading to Sword of Ice)
 */
class PatchOptimizeLakeShrinePlatformsMap : public GamePatch
{
public:
    void alter_world(World& world) override
    {
        for(uint8_t i=0 ; i<10 ; ++i)
            world.map(MAP_LAKE_SHRINE_322)->entity(i)->behavior_id(0);
    }

    void inject_code(md::ROM& rom, World& world) override
    {
        constexpr uint32_t ADDR_NO_BEHAVIOR = 0x0009B458;
        constexpr uint32_t ADDR_BEHAVIOR_FALL = 0x0009B652;

        md::Code func;
        func.movem_to_stack({ reg_D0 }, { reg_A0, reg_A1 });
        {
            func.movew(addr_(0xFF5430), reg_D0);    // D0 = entity underneath player
            func.bmi("player_not_on_entity");
            func.cmpiw(0x500, reg_D0);
            func.bgt("player_not_on_entity");
            {
                // Player is on a platform
                func.lea(0xFF5400, reg_A1);
                func.adda(reg_D0, reg_A1);
                func.cmpil(ADDR_NO_BEHAVIOR, addr_(reg_A1, 0x32));
                func.bne("player_not_on_entity");
                // Player is on a platform that can actually fall, trigger the usual behavior
                func.movel(ADDR_BEHAVIOR_FALL, addr_(reg_A1, 0x32));
                func.movew(0x1D00, addr_(reg_A1, 0x2A));
            }
            func.label("player_not_on_entity");

        }
        func.movem_from_stack({ reg_D0 }, { reg_A0, reg_A1 });
        func.rts();

        uint32_t update_func_addr = rom.inject_code(func);
        world.map(MAP_LAKE_SHRINE_322)->map_update_addr(update_func_addr);
    }
};
