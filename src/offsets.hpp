#pragma once

#include <cstdint>

namespace offsets 
{    
    constexpr uint32_t HUD_TILEMAP = 0x009242;

    constexpr uint32_t MAP_FALL_DESTINATION_TABLE = 0x00A1A8;
    constexpr uint32_t MAP_CLIMB_DESTINATION_TABLE = 0x00A35A;
    constexpr uint32_t MAP_VARIANTS_TABLE = 0x00A3D8;
    constexpr uint32_t MAP_VARIANTS_TABLE_END = 0x00A61C;

    constexpr uint32_t MENU_ITEM_ORDER_TABLE = 0x00D55C;
    constexpr uint32_t MENU_ITEM_ORDER_TABLE_END = 0x00D584;

    constexpr uint32_t PROBABILITY_TABLE = 0x0199D6;
    constexpr uint32_t PROBABILITY_TABLE_END = 0x0199E6;

    constexpr uint32_t MAP_ENTITY_MASKS_TABLE = 0x01A5BA;
    constexpr uint32_t MAP_ENTITY_MASKS_TABLE_END = 0x01A974;

    constexpr uint32_t MAP_CLEAR_FLAGS_TABLE = 0x01A9BE;
    constexpr uint32_t MAP_CLEAR_FLAGS_TABLE_END = 0x01AACC;

    constexpr uint32_t MAP_ENTITIES_OFFSETS_TABLE = 0x01B090;
    constexpr uint32_t ENEMY_STATS_TABLE = 0x01B6F0;
    constexpr uint32_t MAP_ENTITIES_TABLE = 0x01B932;
    constexpr uint32_t MAP_ENTITIES_TABLE_END = 0x022E80;

    constexpr uint32_t ITEM_DATA_TABLE = 0x029304;
    constexpr uint32_t ITEM_NAMES_TABLE = 0x029732;
    constexpr uint32_t ITEM_NAMES_TABLE_END = 0x029A0A;
    
    constexpr uint32_t MAP_BASE_CHEST_ID_TABLE = 0x09E78E;
    constexpr uint32_t CREDITS_TEXT = 0x09ED1A;

    constexpr uint32_t MAP_DATA_TABLE = 0x0A0A12;

    constexpr uint32_t KNL_LIT_ROOM_PALETTE = 0x11CD1C;
    constexpr uint32_t KNL_DARK_ROOM_PALETTE = 0x11CD36;

    constexpr uint32_t MAP_EXITS_TABLE = 0x11CEA2;

    constexpr uint32_t ITEM_SPRITES_TABLE = 0x121578;
}
