#pragma once

#include <landstalker-lib/constants/map_codes.hpp>
#include <landstalker-lib/constants/entity_type_codes.hpp>
#include <landstalker-lib/tools/flag.hpp>

// =========================================================
//      VALUES
// =========================================================

constexpr uint8_t MAX_INDIVIDUAL_JEWELS = 5;


// =========================================================
//      ENTITY TYPES
// =========================================================

constexpr uint8_t ENTITY_NPC_MAGIC_FOX_HIGH_PALETTE = ENTITY_NPC_DUKE_CHAIR;


// =========================================================
//      MAP IDs
// =========================================================

constexpr uint16_t MAP_SECRET_1 = MAP_CRYPT_656;
constexpr uint16_t MAP_SECRET_2 = MAP_INTRO_139;
constexpr uint16_t MAP_SECRET_3 = MAP_INTRO_140;
constexpr uint16_t MAP_SECRET_4 = MAP_INTRO_141;
constexpr uint16_t MAP_SECRET_5 = MAP_INTRO_142;
constexpr uint16_t MAP_SECRET_6 = MAP_INTRO_143;

constexpr uint16_t MAP_GOLAS_HEART_1 = MAP_MERCATOR_CASTLE_ENTRANCE_HALLWAY_57;
constexpr uint16_t MAP_GOLAS_HEART_2 = MAP_MERCATOR_CASTLE_ENTRANCE_HALLWAY_58;
constexpr uint16_t MAP_GOLAS_HEART_3 = MAP_THIEVES_HIDEOUT_TREASURE_ROOM_KAYLA_VARIANT;
constexpr uint16_t MAP_GOLAS_HEART_4 = MAP_MERCATOR_CASTLE_MAIN_HALL_61;
constexpr uint16_t MAP_GOLAS_HEART_5 = MAP_MERCATOR_CASTLE_MAIN_HALL_62;
constexpr uint16_t MAP_GOLAS_HEART_6 = MAP_MERCATOR_CASTLE_MAIN_HALL_60;
constexpr uint16_t MAP_GOLAS_HEART_7 = MAP_MERCATOR_CASTLE_ARMORY_0F_INVADED;
constexpr uint16_t MAP_GOLAS_HEART_8 = 816;
constexpr uint16_t MAP_GOLAS_HEART_9 = MAP_MERCATOR_DOCKS_SUPPLY_SHOP_WITH_ITEMS;
constexpr uint16_t MAP_GOLAS_HEART_STAIRS_1 = 817;
constexpr uint16_t MAP_GOLAS_HEART_STAIRS_2 = 818;
constexpr uint16_t MAP_GOLAS_HEART_STAIRS_3 = 819;
constexpr uint16_t MAP_GOLAS_HEART_FALLING_ROOM = 820;
constexpr uint16_t MAP_GOLAS_HEART_DARK_NOLE_FIGHT = 821;


// =========================================================
//      MEMORY ADDRESSES
// =========================================================

constexpr uint32_t ADDR_ARCHIPELAGO_RECEIVED_ITEM = 0xFF0020;            // .b
constexpr uint32_t ADDR_ARCHIPELAGO_SEED = 0xFF0022;                     // .l
constexpr uint32_t ADDR_ARCHIPELAGO_CURRENT_LOCATION_UUID = 0xFF0026;    // .w
constexpr uint32_t ADDR_ARCHIPELAGO_COMPLETION_BYTE = 0xFF0028;          // .b


constexpr Flag FLAG_ALL_VALID_EQUIPMENTS = Flag(0x2C, 0);   // 0xFF102C
constexpr Flag FLAG_FOUND_GNOME_1 = Flag(0x2C, 1);
constexpr Flag FLAG_FOUND_GNOME_2 = Flag(0x2C, 2);
constexpr Flag FLAG_FOUND_GNOME_3 = Flag(0x2C, 3);
constexpr Flag FLAG_FOUND_GNOME_4 = Flag(0x2C, 4);
constexpr Flag FLAG_SOLVED_ROCKS_RIDDLE = Flag(0x3E, 0);

constexpr Flag FLAG_USED_BLUE_RIBBON = Flag(0x7D, 0);
constexpr uint32_t ADDR_CURRENT_RECEIVED_ITEM_INDEX = 0xFF107E;          // .w

// Flags related to Gola's Heart room completion
constexpr Flag FLAG_CLEARED_GH_3 = Flag(0xCE, 0);
constexpr Flag FLAG_CLEARED_GH_4 = Flag(0xCE, 1);
constexpr Flag FLAG_CLEARED_GH_5 = Flag(0xCE, 2);
constexpr Flag FLAG_CLEARED_GH_6 = Flag(0xCE, 3);
