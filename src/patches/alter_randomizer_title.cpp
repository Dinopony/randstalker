#include <landstalker_lib/md_tools.hpp>
#include "../assets/edited_title_tileset.bin.hxx"

void alter_randomizer_title(md::ROM& rom)
{
    // Change "THE TREASURES OF KING NOLE" to "RANDOMIZER"
    rom.set_bytes(0x3BCAE, EDITED_TITLE_TILESET, EDITED_TITLE_TILESET_SIZE);
}
