#pragma once

#include "landstalker_lib/patches/game_patch.hpp"

#include "landstalker_lib/model/world.hpp"
#include "landstalker_lib/model/item.hpp"
#include "landstalker_lib/constants/item_codes.hpp"

/**
 * This patch handles most things specifically related to Archipelago multiworld.
 *
 */
class PatchHandleArchipelago : public GamePatch
{
private:
    static constexpr uint32_t ADDR_RECEIVED_ITEM = 0xFF0020;
    static constexpr uint32_t ADDR_SEED = 0xFF0022;
    static constexpr uint32_t ADDR_CURRENT_RECEIVED_ITEM_INDEX = 0xFF107E;

    uint32_t _seed;

public:
    explicit PatchHandleArchipelago(const RandomizerOptions& options) :
        _seed(options.seed())
    {}

    void inject_code(md::ROM& rom, World& world) override
    {
        // Right before SEGA logo display, call the specific initialization function
        uint32_t func_boot_init_addr = this->inject_func_boot_init(rom);
        rom.set_long(0x38616, func_boot_init_addr);

        // Change the message when obtaining an Archipelago item in chest
        add_proc_handle_archipelago_items_in_chests(rom);
    }

private:
    uint32_t inject_func_boot_init(md::ROM& rom) const
    {
        md::Code func;
        {
            func.moveb(0xFF, addr_(ADDR_RECEIVED_ITEM));
            func.movel(_seed, addr_(ADDR_SEED));
            func.movew(0x0000, addr_(ADDR_CURRENT_RECEIVED_ITEM_INDEX));
        }
        func.jsr(0x230); // call j_DisableDisplayAndInts whose call was replaced by this function
        func.rts();
        return rom.inject_code(func);
    }

    static void add_proc_handle_archipelago_items_in_chests(md::ROM& rom)
    {
        md::Code proc;
        {
            proc.cmpib(ITEM_LIFESTOCK, reg_D0);
            proc.bne("not_lifestock");
            {
                proc.jmp(0x7132);
            }
            proc.label("not_lifestock");
            proc.cmpib(ITEM_ARCHIPELAGO, reg_D0);
            proc.bne("not_archipelago");
            {
                proc.movem_to_stack({ reg_D0, reg_D1 }, {});
                proc.jsr(0x9B00C);                  // Open chest
                proc.trap(0x00, { 0x00, 0x05 });    // Play chest jingle
                proc.movew(0x001E, reg_D0);         // "Got an Archipelago Item"
                proc.jsr(0x22E90);                  // j_PrintString
                proc.jsr(0x852);                    // RestoreBGM
                proc.movem_from_stack({ reg_D0, reg_D1 }, {});
                proc.jmp(0x7190);
            }
            proc.label("not_archipelago");
        }
        proc.jmp(0x70DC);

        uint32_t addr = rom.inject_code(proc);
        rom.set_code(0x70D6, md::Code().jmp(addr));
    }
};
