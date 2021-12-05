#include <md_tools.hpp>

#include "assets/fixed_hud_tilemap.bin.hxx"
#include "../../constants/offsets.hpp"

/**
 * In the original game, the HUD tilemap has a problem on the zero character, which is the only one not having a shadow.
 * This is fixed in this edited version of the HUD tilemap.
 */
void fix_hud_tilemap(md::ROM& rom)
{
    for(uint32_t i=0 ; i<FIXED_HUD_TILEMAP_SIZE ; ++i)
        rom.set_byte(offsets::HUD_TILEMAP+i, FIXED_HUD_TILEMAP[i]);
}