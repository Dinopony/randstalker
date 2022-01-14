#include <landstalker_lib/md_tools.hpp>

static void alter_prospero_dialogues(md::ROM& rom)
{
    // Change a big story-dependant dialogue branch into one dialogue
    rom.set_code(0x2617A, md::Code().trap(1));
    rom.set_word(0x2617C, 0x19A5);
    rom.set_code(0x2617E, md::Code().rts());
    rom.mark_empty_chunk(0x26180, 0x261A8);

    // Change the unique dialogue script
    rom.set_word(0x27B20, 0x8177); // Message 0x1C4
    rom.set_word(0x27B22, 0xE178); // Message 0x1C5

    // Empty unused cutscene scripts
    rom.mark_empty_chunk(0x282D4, 0x282FE);
    // Empty unused dialogue scripts
    rom.mark_empty_chunk(0x27B24, 0x27B52);
}

static void alter_destel_boatmaker_dialogues(md::ROM& rom)
{
    // Change the story-dependant dialogue branch into one dialogue
    rom.set_code(0x2711E, md::Code().trap(1));
    rom.set_word(0x27120, 0x1153);
    rom.set_code(0x27122, md::Code().rts());

    // Change which text line ends the dialogue
    rom.set_byte(0x28274, 0x84);
    rom.set_byte(0x28276, 0xE4);
}

void alter_hint_provider_dialogues(md::ROM& rom)
{
    alter_prospero_dialogues(rom);
    alter_destel_boatmaker_dialogues(rom);
}
