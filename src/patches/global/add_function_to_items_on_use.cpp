#include <md_tools.hpp>

#include "../../constants/item_codes.hpp"

static uint32_t make_record_book_save_on_use(md::ROM& rom, bool consumable_record_book)
{
    constexpr uint32_t MAP_ENTRANCE_POSITION_ADDRESS = 0xFF0018;

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
    func_use_record_book.jsr(0x1592); // "func_save_game"

    func_use_record_book.movew(reg_D0, addr_(0xFF1204));
    func_use_record_book.movew(reg_D1, addr_(0xFF5400));
    func_use_record_book.movem_from_stack({ reg_D0, reg_D1 }, {});

    func_use_record_book.rts();
    return rom.inject_code(func_use_record_book, "func_use_record_book");
}

static void make_spell_book_warp_to_start(md::ROM& rom)
{
    uint16_t spawnX = rom.get_byte(0x0027FD);
    uint16_t spawnZ = rom.get_byte(0x002805);
    uint16_t spawnPosition = (spawnX << 8) + spawnZ;
    uint16_t spawnMapID = rom.get_word(0x0027F4);

    // ------------- New spell book teleport function -------------
    md::Code spellBookFunction;
    spellBookFunction.movem_to_stack({ reg_D0_D7 }, { reg_A0_A6 });
    spellBookFunction.movew(spawnPosition, addr_(0xFF5400));
    spellBookFunction.movew(0x0708, addr_(0xFF5402)); // Reset subtiles position
    spellBookFunction.trap(0, { 0x00, 0x4D });
    spellBookFunction.jsr(0x44C);
    spellBookFunction.movew(spawnMapID, reg_D0); // Set MapID to spawn map
    spellBookFunction.movew(0x0000, addr_(0xFF5412)); // Reset player height
    spellBookFunction.moveb(0x00, addr_(0xFF5422)); // Reset ground height
    spellBookFunction.moveb(0x00, addr_(0xFF5439)); // ^
    spellBookFunction.jsr(0x1586E);
    spellBookFunction.jsr(0x434);
    spellBookFunction.clrb(reg_D0);
    spellBookFunction.jsr(0x2824);
    spellBookFunction.jsr(0x410);
    spellBookFunction.jsr(0x8EB4);
    spellBookFunction.movem_from_stack({ reg_D0_D7 }, { reg_A0_A6 });
    spellBookFunction.rts();
    rom.inject_code(spellBookFunction, "func_spell_book");

    // ------------- Extended item post use table -------------
    uint32_t newPostUseTableAddr = rom.reserve_data_block(0x2A+6, "new_post_use_table");
    // PostUseGarlic
    rom.set_long(newPostUseTableAddr, 0x00008BE8); 
    rom.set_word(newPostUseTableAddr+4, 0x9BFF);
    newPostUseTableAddr += 6;
    // PostUseEinsteinWhistle
    rom.set_long(newPostUseTableAddr, 0x00008BF2);
    rom.set_word(newPostUseTableAddr+4, 0xA0FF);
    newPostUseTableAddr += 6;
    // PostUsePostUseGolasEye
    rom.set_long(newPostUseTableAddr, 0x00008C7C); 
    rom.set_word(newPostUseTableAddr+4, 0xABFF);
    newPostUseTableAddr += 6;
    // PostUseIdolStone
    rom.set_long(newPostUseTableAddr, 0x00008C8E); 
    rom.set_word(newPostUseTableAddr+4, 0xB1FF);
    newPostUseTableAddr += 6;
    // PostUseKey
    rom.set_long(newPostUseTableAddr, 0x00008CB6); 
    rom.set_word(newPostUseTableAddr+4, 0xB2FF);
    newPostUseTableAddr += 6;
    // PostUseShortcake
    rom.set_long(newPostUseTableAddr, 0x00008CF6); 
    rom.set_word(newPostUseTableAddr+4, 0xB6FF);
    newPostUseTableAddr += 6;
    // PostUseSpellbook
    rom.set_long(newPostUseTableAddr, rom.stored_address("func_spell_book")); 
    rom.set_word(newPostUseTableAddr+4, 0xA4FF);
    newPostUseTableAddr += 6;
    // End
    rom.set_long(newPostUseTableAddr, 0xFFFFFFFF); 
    rom.set_word(newPostUseTableAddr+4, 0xFFFF);

    // ------------- New function to read the new table -------------
    md::Code newPostUseTableLookupProc;
    newPostUseTableLookupProc.moveb(addr_(0x00FF1152), reg_D0);
    newPostUseTableLookupProc.lea(rom.stored_address("new_post_use_table"), reg_A0);
    newPostUseTableLookupProc.label("newPostUseTableLookupProc_loop");
    newPostUseTableLookupProc.moveb(addr_(reg_A0, 0x04), reg_D2);
    newPostUseTableLookupProc.cmpib(0xFF, reg_D2);
    newPostUseTableLookupProc.beq(7);
    newPostUseTableLookupProc.cmpb(reg_D0, reg_D2);
    newPostUseTableLookupProc.beq(3);
    newPostUseTableLookupProc.addql(0x06, reg_A0);
    newPostUseTableLookupProc.bra("newPostUseTableLookupProc_loop");
    newPostUseTableLookupProc.movel(addr_(reg_A0), reg_A0);
    newPostUseTableLookupProc.jmp(addr_(reg_A0));
    newPostUseTableLookupProc.rts();
    uint32_t newPostUseTableLookupProcAddr = rom.inject_code(newPostUseTableLookupProc);

    // ------------- JMP to override old lookup function by the new one -------------
    md::Code postUseTableInjectorCall;
    postUseTableInjectorCall.jmp(newPostUseTableLookupProcAddr);
    rom.set_code(0x8BC8, postUseTableInjectorCall);

    // Replace "consume Spell Book" by "trigger post use effect"
    rom.set_long(0x88C6, 0x600002EA); // bra loc_8BB2 >>> Mark used item as "has post use effect"
}

void add_functions_to_items_on_use(md::ROM& rom, bool consumable_record_book)
{
    make_spell_book_warp_to_start(rom);
    uint32_t func_use_record_book_addr = make_record_book_save_on_use(rom, consumable_record_book);

    // ------------- Extended item handling function -------------

    md::Code funcExtendedItemHandling;

    funcExtendedItemHandling.cmpib(ITEM_RECORD_BOOK, reg_D0);
    funcExtendedItemHandling.bne(3);
        funcExtendedItemHandling.jsr(func_use_record_book_addr);
        funcExtendedItemHandling.rts();
    funcExtendedItemHandling.cmpib(ITEM_SPELL_BOOK, reg_D0);
    funcExtendedItemHandling.bne(3);
        funcExtendedItemHandling.jsr(0xDC1C); // "func_abracadabra"
        funcExtendedItemHandling.rts();
    funcExtendedItemHandling.cmpib(ITEM_LITHOGRAPH, reg_D0);
    funcExtendedItemHandling.bne(3);
        funcExtendedItemHandling.movew(0x21, reg_D0); // Read game string 0x21
        funcExtendedItemHandling.jsr(0x22E90);
    funcExtendedItemHandling.rts();
    
    uint32_t funcExtendedItemHandlingAddr = rom.inject_code(funcExtendedItemHandling);

    rom.set_code(0x00DBA8, md::Code().jsr(funcExtendedItemHandlingAddr).nop(4));

    // -------------------- Other modifications ---------------------

    // To remove the "Nothing happened..." text, the item must be put in a list which has a finite size.
    // We replace the Blue Ribbon (0x18) by the Record Book (0x23) to do so.
    rom.set_byte(0x008642, 0x23);
    // Same for Lithograph (0x27) remplacing Lantern (0x1A)
    rom.set_word(0x008647, 0x6627);
}
