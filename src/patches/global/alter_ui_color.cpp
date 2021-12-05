#include <md_tools.hpp>

/// ui_color follows the (0BGR) format used by palettes
void alter_ui_color(md::ROM& rom, uint16_t ui_color)
{
    rom.set_word(0xF6D0, ui_color);
    rom.set_word(0xFB36, ui_color);
    rom.set_word(0x903C, ui_color);
}