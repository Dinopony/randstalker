#include <md_tools.hpp>

#include "../../constants/offsets.hpp"
#include "../../constants/item_codes.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////////
//      UI CHANGES
////////////////////////////////////////////////////////////////////////////////////////////////////////

void alter_item_order_in_menu(md::ROM& rom)
{
    std::vector<uint8_t> itemOrder = {
        ITEM_RECORD_BOOK,   ITEM_SPELL_BOOK,
        ITEM_EKEEKE,        ITEM_DAHL,
        ITEM_GOLDEN_STATUE, ITEM_GAIA_STATUE,
        ITEM_RESTORATION,   ITEM_DETOX_GRASS,
        ITEM_MIND_REPAIR,   ITEM_ANTI_PARALYZE,
        ITEM_KEY,           ITEM_IDOL_STONE,
        ITEM_GARLIC,        ITEM_LOGS,
        ITEM_GOLA_EYE,      ITEM_GOLA_NAIL,
        ITEM_GOLA_HORN,     ITEM_GOLA_FANG,
        ITEM_DEATH_STATUE,  ITEM_CASINO_TICKET,
        ITEM_SHORT_CAKE,    ITEM_PAWN_TICKET,
        ITEM_ORACLE_STONE,  ITEM_LITHOGRAPH,
        ITEM_RED_JEWEL,     ITEM_PURPLE_JEWEL,
        ITEM_GREEN_JEWEL,   ITEM_BLUE_JEWEL,
        ITEM_YELLOW_JEWEL,  ITEM_SAFETY_PASS,
        ITEM_AXE_MAGIC,     ITEM_ARMLET,
        ITEM_SUN_STONE,     ITEM_BUYER_CARD,
        ITEM_LANTERN,       ITEM_EINSTEIN_WHISTLE,
        ITEM_STATUE_JYPTA,  ITEM_BELL,
        ITEM_BLUE_RIBBON,   0xFF
    };

    for (int i = 0; offsets::MENU_ITEM_ORDER_TABLE + i < offsets::MENU_ITEM_ORDER_TABLE_END; ++i)
        rom.set_byte(offsets::MENU_ITEM_ORDER_TABLE + i, itemOrder[i]);
}

/**
 * Make the effect of Statue of Gaia and Sword of Gaia way faster, because reasons.
 */
void quicken_gaia_effect(md::ROM& rom)
{
    constexpr uint8_t SPEEDUP_FACTOR = 3;

    rom.set_word(0x1686C, rom.get_word(0x1686C) * SPEEDUP_FACTOR);
    rom.set_word(0x16878, rom.get_word(0x16878) * SPEEDUP_FACTOR);
    rom.set_word(0x16884, rom.get_word(0x16884) * SPEEDUP_FACTOR);
}
