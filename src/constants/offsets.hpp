#pragma once

#include <cstdint>

namespace offsets 
{    
    constexpr uint32_t HUD_TILEMAP = 0x009242;

    constexpr uint32_t MAP_FALL_DESTINATION_TABLE = 0x00A1A8;
    constexpr uint32_t MAP_FALL_DESTINATION_TABLE_END = 0x00A35A;
    constexpr uint32_t MAP_CLIMB_DESTINATION_TABLE = MAP_FALL_DESTINATION_TABLE_END;
    constexpr uint32_t MAP_CLIMB_DESTINATION_TABLE_END = 0x00A3D8;
    constexpr uint32_t MAP_VARIANTS_TABLE = MAP_CLIMB_DESTINATION_TABLE_END;
    constexpr uint32_t MAP_VARIANTS_TABLE_END = 0x00A61C;

    constexpr uint32_t MAP_VISITED_FLAG_TABLE = 0x00C74E;
    constexpr uint32_t MAP_VISITED_FLAG_TABLE_END = 0x00CDAE;

    constexpr uint32_t MENU_ITEM_ORDER_TABLE = 0x00D55C;
    constexpr uint32_t MENU_ITEM_ORDER_TABLE_END = 0x00D584;

    constexpr uint32_t PROBABILITY_TABLE = 0x0199D6;
    constexpr uint32_t PROBABILITY_TABLE_END = 0x0199E6;

    constexpr uint32_t MAP_ENTITY_MASKS_TABLE = 0x01A5BA;
    constexpr uint32_t MAP_ENTITY_MASKS_TABLE_END = 0x01A974;

    constexpr uint32_t MAP_CLEAR_FLAGS_TABLE = 0x01A9BE;
    constexpr uint32_t MAP_CLEAR_FLAGS_TABLE_END = 0x01AACC;

    constexpr uint32_t PERSISTENCE_FLAGS_TABLE = 0x1AAF6;
    constexpr uint32_t SACRED_TREES_PERSISTENCE_FLAGS_TABLE = 0x1ABA8;
    constexpr uint32_t SACRED_TREES_PERSISTENCE_FLAGS_TABLE_END = 0x1ABF2;
    
    constexpr uint32_t MAP_ENTITIES_OFFSETS_TABLE = 0x01B090;
    
    constexpr uint32_t ENEMY_STATS_TABLE = 0x01B6F0;
    constexpr uint32_t ENEMY_STATS_TABLE_END = 0x01B932;
    
    constexpr uint32_t MAP_ENTITIES_TABLE = 0x01B932;
    constexpr uint32_t MAP_ENTITIES_TABLE_END = 0x022E80;

    constexpr uint32_t DIALOGUE_TABLE = 0x0256D6;
    constexpr uint32_t DIALOGUE_TABLE_END = 0x025B0C;

    constexpr uint32_t ITEM_DATA_TABLE = 0x029304;
    constexpr uint32_t ITEM_NAMES_TABLE = 0x029732;
    constexpr uint32_t ITEM_NAMES_TABLE_END = 0x029A0A;

    constexpr uint32_t LITHOGRAPH_TILES = 0x0389D3;
    constexpr uint32_t LITHOGRAPH_TILES_END = 0x039762;

    constexpr uint32_t BEHAVIOR_OFFSETS_TABLE = 0x09B058; 
    constexpr uint32_t BEHAVIOR_TABLE = 0x09B458;
    constexpr uint32_t BEHAVIOR_TABLE_END = 0x09E75E;

    constexpr uint32_t MAP_BASE_CHEST_ID_TABLE = 0x09E78E;
    constexpr uint32_t CREDITS_TEXT = 0x09ED1A;

    constexpr uint32_t MAP_DATA_TABLE = 0x0A0A12;

    constexpr uint32_t KNL_LIT_ROOM_PALETTE = 0x11CD1C;
    constexpr uint32_t KNL_DARK_ROOM_PALETTE = 0x11CD36;

    constexpr uint32_t MAP_CONNECTIONS_TABLE = 0x11CEA2;
    constexpr uint32_t MAP_CONNECTIONS_TABLE_END = 0x11EA64;

    constexpr uint32_t ITEM_SPRITES_TABLE = 0x121578;
}
