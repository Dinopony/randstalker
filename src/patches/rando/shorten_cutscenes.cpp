#include <md_tools.hpp>

static void shorten_mir_death_cutscene(md::ROM& rom)
{
    // Cut the cutscene script
    rom.set_word(0x2872C, 0xE646);
    rom.set_word(0x2873C, 0xE64B);
}

static void shorten_mir_cutscene_after_lake_shrine(md::ROM& rom)
{
    // Cut the cutscene script
    rom.set_word(0x28A44, 0xE739);
}

static void shorten_arthur_dialogue(md::ROM& rom)
{
    for(uint32_t addr=0x28154 ; addr <= 0x2815D ; addr += 0x2)
        rom.set_word(addr, 0x0000);
}

void shorten_cutscenes(md::ROM& rom)
{
    shorten_mir_death_cutscene(rom);
    shorten_mir_cutscene_after_lake_shrine(rom);
    shorten_arthur_dialogue(rom);
}