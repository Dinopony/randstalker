#pragma once

#include <landstalker-lib/patches/game_patch.hpp>
#include <landstalker-lib/constants/offsets.hpp>
#include <landstalker-lib/constants/item_codes.hpp>

#include "../../logic_model/hint_source.hpp"
#include "../../logic_model/randomizer_world.hpp"

class PatchRecordBookSaveOnUse : public GamePatch
{
private:
    bool _consumable_record_book = false;

    static constexpr uint32_t MAP_ENTRANCE_POSITION_ADDRESS = 0xFF0018;

public:
    explicit PatchRecordBookSaveOnUse(bool consumable_record_book) : _consumable_record_book(consumable_record_book) {}

    void inject_code(md::ROM& rom, World& world) override
    {
        // Inject a function on map load which stores where the charracter was on map entrance
        md::Code func;
        {
            func.jsr(0x1A510); // CheckSacredTreeFlags
            func.movew(addr_(0xFF1204), addr_(MAP_ENTRANCE_POSITION_ADDRESS));
            func.movew(addr_(0xFF5400), addr_(MAP_ENTRANCE_POSITION_ADDRESS + 2));
        }
        func.rts();

        uint32_t func_addr = rom.inject_code(func);
        rom.set_code(0x19B5C, md::Code().jmp(func_addr));

        // Inject the actual saving function which uses the stored position
        uint32_t func_use_record_book = inject_use_record_book_function(rom);
        world.item(ITEM_RECORD_BOOK)->pre_use_address(func_use_record_book);
    }

private:
    uint32_t inject_use_record_book_function(md::ROM& rom) const
    {
        // -------- Function to save game using record book --------
        // On record book use, set stored position and map as current to fool the save_game function, then call it
        // to save the game with this map and position. Then, restore Nigel's position and map as if nothing happened.

        md::Code func_use_record_book;
        func_use_record_book.movem_to_stack({ reg_D0, reg_D1 }, {});

        if(_consumable_record_book)
        {
            func_use_record_book.moveb(reg_D0, addr_(0xFF1152));
            func_use_record_book.jsr(0x8B98); // ConsumeItem
        }

        func_use_record_book.movew(addr_(0xFF1204), reg_D0);
        func_use_record_book.movew(addr_(0xFF5400), reg_D1);

        func_use_record_book.movew(addr_(MAP_ENTRANCE_POSITION_ADDRESS), addr_(0xFF1204));
        func_use_record_book.movew(addr_(MAP_ENTRANCE_POSITION_ADDRESS+2), addr_(0xFF5400));

        func_use_record_book.trap(0, { 0x00, 0x07 }); // Play save game music
        func_use_record_book.jsr(0x29046); // Sleep_0 for 0x17 frames
        func_use_record_book.add_word(0x0000);
        func_use_record_book.jsr(0x852); // Restore BGM

        func_use_record_book.jsr(0x1592); // Save game to SRAM

        func_use_record_book.movew(reg_D0, addr_(0xFF1204));
        func_use_record_book.movew(reg_D1, addr_(0xFF5400));
        func_use_record_book.movem_from_stack({ reg_D0, reg_D1 }, {});

        func_use_record_book.jmp(offsets::PROC_ITEM_USE_RETURN_SUCCESS);

        return rom.inject_code(func_use_record_book);
    }
};
