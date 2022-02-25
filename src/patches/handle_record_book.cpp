#include <landstalker_lib/md_tools.hpp>
#include <landstalker_lib/constants/offsets.hpp>
#include <landstalker_lib/constants/item_codes.hpp>
#include <landstalker_lib/model/world.hpp>
#include <landstalker_lib/model/item.hpp>

constexpr uint32_t MAP_ENTRANCE_POSITION_ADDRESS = 0xFF0018;

static void store_position_on_transitions(md::ROM& rom)
{
    // =========== Procedures to store map and position after last map transition ===========
    md::Code func_store_position;
    func_store_position.movew(addr_(0xFF1204), addr_(MAP_ENTRANCE_POSITION_ADDRESS));
    func_store_position.movew(addr_(0xFF5400), addr_(MAP_ENTRANCE_POSITION_ADDRESS+2));
    func_store_position.rts();
    uint32_t func_store_position_addr = rom.inject_code(func_store_position);

    // Inject this func_store_position on every map change by hooking it into the BellCheck function.
    md::Code bell_check_injector;
    bell_check_injector.jsr(func_store_position_addr);
    bell_check_injector.jsr(0x22ED0); // Call j_GetItemQtyAndMaxQty because we replaced it to hook this function
    bell_check_injector.rts();
    uint32_t bell_check_injector_addr = rom.inject_code(bell_check_injector);
    rom.set_long(0x669E, bell_check_injector_addr);

    // Game load transition injection
    md::Code func_load_game_and_store_pos;
    func_load_game_and_store_pos.jsr(0x15C2); // "func_load_game"
    func_load_game_and_store_pos.jsr(func_store_position_addr);
    func_load_game_and_store_pos.rts();
    uint32_t func_load_game_and_store_pos_addr = rom.inject_code(func_load_game_and_store_pos);
    rom.set_code(0xEF46, md::Code().jsr(func_load_game_and_store_pos_addr));
}

static uint32_t inject_use_record_book_function(md::ROM& rom, bool consumable_record_book)
{
    // -------- Function to save game using record book --------
    // On record book use, set stored position and map as current to fool the save_game function, then call it
    // to save the game with this map and position. Then, restore Nigel's position and map as if nothing happened.

    md::Code func_use_record_book;
    func_use_record_book.movem_to_stack({ reg_D0, reg_D1 }, {});

    if(consumable_record_book)
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

void handle_record_book_save(md::ROM& rom, World& world, bool consumable_record_book)
{
    store_position_on_transitions(rom);
    uint32_t func_use_record_book = inject_use_record_book_function(rom, consumable_record_book);
    world.item(ITEM_RECORD_BOOK)->pre_use_address(func_use_record_book);
}
