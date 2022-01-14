#include <landstalker_lib/md_tools.hpp>
#include <landstalker_lib/tools/huffman/symbols.hpp>

/**
 * Replace "Copy" behavior in menu by an injected function showing seed hash
 */
void replace_copy_save_by_show_hash(md::ROM& rom, const std::string& seed_hash_sentence)
{
    md::Code show_hash_func;
    show_hash_func.cmpib(1, reg_D0);
    show_hash_func.bne("hash_not_selected");
        show_hash_func.jsr(0xF5F4); // ClearTextBuffer
        show_hash_func.movew(0x4D, reg_D1);
        show_hash_func.jsr(0xF618); // j_j_LoadUncompressedString
        show_hash_func.jsr(0xB5E); // WaitUntilVBlank
        show_hash_func.jsr(0xF5DC); // DMACopyTextBuffer
        show_hash_func.moveb(0x00, addr_(0xFF0556));
        show_hash_func.jsr(0x10C6); // WaitForNextButtonPress
        show_hash_func.jmp(0x00EEF6);
        show_hash_func.add_bytes({ 0x3D, 0x7C, 0xFF, 0xFF, 0xFF, 0xFC });
    show_hash_func.label("hash_not_selected");
    show_hash_func.jmp(0x00EF00);

    uint32_t addr = rom.inject_code(show_hash_func);
    rom.set_code(0xEEFA, md::Code().jmp(addr));

    // Replace "Copy" string by "Hash"
    rom.set_bytes(0x29A20, Symbols::bytes_for_symbols("Hash"));

    // Replace "Massan" by an empty string, and make this string the one shown for every map in the game
    rom.set_word(0x29A4B, 0x0100);
    rom.set_word(0x294E8, 0xFFFF);

    // Put hash sentence as string 4D
    rom.set_byte(0x29A4D, (uint8_t)seed_hash_sentence.size());
    rom.set_bytes(0x29A4E, Symbols::bytes_for_symbols(seed_hash_sentence));
}
