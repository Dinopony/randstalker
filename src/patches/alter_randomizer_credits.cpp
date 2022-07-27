#include <landstalker_lib/md_tools.hpp>

#include <landstalker_lib/constants/offsets.hpp>
#include <landstalker_lib/exceptions.hpp>

void alter_randomizer_credits(md::ROM& rom)
{
    const std::vector<uint8_t> credits_text = rom.get_bytes(offsets::CREDITS_TEXT, offsets::CREDITS_TEXT_END);
    std::vector<uint8_t> new_credits_text;

    new_credits_text.insert(new_credits_text.begin(), credits_text.begin(), credits_text.begin() + 0x75);

    // Change "LANDSTALKER" to "RANDSTALKER"
    new_credits_text[0x2] = 0x13;

    std::vector<uint8_t> landstalker_randomizer = {
        0x27, 0x1C, 0x29, 0x1F, 0x2E, 0x2F, 0x1C, 0x27, 0x26, 0x20, 0x2D, 0x80, // "landstalker "
        0x2D, 0x1C, 0x29, 0x1F, 0x2A, 0x28, 0x24, 0x35, 0x20, 0x2D              // "randomizer"
    };
    std::copy(landstalker_randomizer.begin(), landstalker_randomizer.end(), new_credits_text.begin() + 0x14);

    // Widen the space between the end of the cast and the beginning of the rando staff
    new_credits_text[0x5C] = 0x0F;

    // Replace the "game cast" (useless list of game characters) by the randomizer credits
    std::vector<uint8_t> rando_credits = {
        // " R A N D O M I Z E R "
        0x08, 0xFF, 0x82, 
        0x80, 0x13, 0x80, 0x02, 0x80, 0x0F, 0x80, 0x05, 0x80, 0x10, 0x80, 0x0E, 0x80, 0x0A, 0x80, 0x1B, 0x80, 0x06, 0x80, 0x13, 0x80, 0x00, 
        // "developed by"
        0x04, 0xFF, 0x81, 
        0x1F, 0x20, 0x31, 0x20, 0x27, 0x2A, 0x2B, 0x20, 0x1F, 0x80, 0x1D, 0x34, 0x00,
        // "DINOPONY"
        0x03, 0xFF,
        0x05, 0x0A, 0x0F, 0x10, 0x11, 0x10, 0x0F, 0x1A, 0x00,
        // "WIZ"
        0x08, 0xFF,
        0x18, 0x0A, 0x1B, 0x00,
        // "frontend by"
        0x04, 0xFF, 0x81,
        0x21, 0x2D, 0x2A, 0x29, 0x2F, 0x20, 0x29, 0x1F, 0x80, 0x1D, 0x34, 0x00,
        // "HAWKREX"
        0x03, 0xFF,
        0x09, 0x02, 0x18, 0x0C, 0x13, 0x06, 0x19, 0x00,
        // "DILANDAU"
        0x08, 0xFF,
        0x05, 0x0A, 0x0D, 0x02, 0x0F, 0x05, 0x02, 0x16, 0x00,
        // testing
        0x04, 0xFF, 0x81,
        0x2F, 0x20, 0x2E, 0x2F, 0x24, 0x29, 0x22, 0x00,
        // SAGAZ
        0x03, 0xFF,
        0x14, 0x02, 0x08, 0x02, 0x1B, 0x00,
        // STIK
        0x03, 0xFF,
        0x14, 0x15, 0x0A, 0x0C, 0x00,
        // LANDRYLE
        0x08, 0xFF,
        0x0D, 0x02, 0x0F, 0x05, 0x13, 0x1A, 0x0D, 0x06, 0x00
    };
    new_credits_text.insert(new_credits_text.end(), rando_credits.begin(), rando_credits.end());

    // Add the rest of the original game credits
    new_credits_text.insert(new_credits_text.end(), credits_text.begin() + 0x1D3, credits_text.begin() + 0x929);

    if(new_credits_text.size() > credits_text.size())
        throw LandstalkerException("New credits cannot be bigger than original credits");

    // Clear old credits bytes, and replace them with the new credits
    for(uint32_t addr = offsets::CREDITS_TEXT ; addr < offsets::CREDITS_TEXT_END ; ++addr)
        rom.set_byte(addr, 0x00);
    rom.set_bytes(offsets::CREDITS_TEXT, new_credits_text);

    // Credits debug code, uncomment to launch credits when opening inventory menu
    // rom.set_code(0x7654, md::Code().jsr(0xEAEA).rts());
}

void alter_credits_palette_on_specific_condition(md::ROM& rom)
{
    std::vector<uint8_t> alternate_credits_palette = {
        0x00, 0x00,
        0x0E, 0x8E,
        0x0C, 0x6C,
        0x0A, 0x4A
    };
    uint32_t alternate_palette_addr = rom.inject_bytes(alternate_credits_palette);

    md::Code func;
    func.lea(0xFF0080, reg_A1); // instruction erased by the jsr
    func.btst(1, addr_(0xFF104C));
    func.beq("ret");
    func.lea(alternate_palette_addr, reg_A0); // Use alternate credits palette if condition is met
    func.label("ret");
    func.rts();
    uint32_t func_addr = rom.inject_code(func);

    rom.set_code(0x9EC5A, md::Code().jsr(func_addr));
}
