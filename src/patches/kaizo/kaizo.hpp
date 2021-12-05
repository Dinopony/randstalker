#pragma once

#include <md_tools.hpp>

#include "../../world_model/world.hpp"
#include "../../world_model/map.hpp"
#include "../../world_model/map_palette.hpp"
#include "../../world_model/entity.hpp"
#include "../../world_model/entity_type.hpp"
#include "../../world_model/spawn_location.hpp"
#include "../../world_model/item_source.hpp"

#include "../../constants/entity_type_codes.hpp"
#include "../../constants/map_codes.hpp"
#include "../../constants/item_codes.hpp"
#include "../../constants/offsets.hpp"

////// EDIT CHAPTER FUNCTIONS ///////////////////////////////////////////////////////////////////

void patch_engine_for_kaizo(World& world, md::ROM& rom);
void edit_safety_pass_arc(World& world, md::ROM& rom);
void edit_mercator(World& world, md::ROM& rom);
void edit_mercator_castle(World& world, md::ROM& rom);

////// KAIZO SPECIFIC EDIT TOOLS ///////////////////////////////////////////////////////////////////

void batch_add_entities(Map* map, const std::vector<Position>& positions, Entity::Attributes base_attributes,  bool use_tiles_from_first = true);
void wipe_map_variants(Map* map);
void wipe_map_variants(World& world, const std::vector<uint16_t>& map_ids);
void make_entity_visible_with_golas_eye(Entity* entity);
void create_chest_item_source(World& world, uint8_t chest_id, uint8_t item_id);

////// KAIZO CONSTANT VALUES ///////////////////////////////////////////////////////////////////

constexpr uint16_t SWORD_0_BASE_POWER = 20;
constexpr uint16_t SWORD_1_BASE_POWER = 25;
constexpr uint16_t SWORD_2_BASE_POWER = 30;
constexpr uint16_t SWORD_3_BASE_POWER = 35;
constexpr uint16_t SWORD_4_BASE_POWER = 40;

////// ENTITY TYPE ALIASES ///////////////////////////////////////////////////////////////////

constexpr uint8_t POWER_GLOVE_BLOCKER_TYPE = ENTITY_LARGE_BLUE_BOULDER;
constexpr uint8_t QUARTZ_SWORD_BLOCKER_TYPE = ENTITY_GOLD_GOLEM_STATUE;

////// USEFUL BEHAVIOR IDS ///////////////////////////////////////////////////////////////////

constexpr uint16_t BEHAVIOR_DISAPPEAR_ON_ALL_ENEMIES_BEATEN = 106;
constexpr uint16_t BEHAVIOR_ALLOW_APPEAR_AFTER_PLAYER_MOVED_AWAY = 785;
constexpr uint16_t BEHAVIOR_BUTTON_SET_PERSISTENCE_FLAG = 544;
constexpr uint16_t BEHAVIOR_INVISIBLE_REVEALABLE_BY_GOLAS_EYE = 531;

////// FLAGS /////////////////////////////////////////////////////////////////// 

constexpr Flag FLAG_ALWAYS_TRUE                      = Flag(0x00, 0);
constexpr Flag FLAG_HELGA_TALKED_ABOUT_WORM          = Flag(0x04, 4);
constexpr Flag FLAG_KAYLA_BATHROOM_BUTTON_PRESSED    = Flag(0x06, 0);
constexpr Flag FLAG_DEXTER_ROOM_BUTTON_PRESSED       = Flag(0x06, 1);
constexpr Flag FLAG_ZAK_ROOM_BUTTON_PRESSED          = Flag(0x06, 2);
constexpr Flag FLAG_SAFETY_PASS_OWNED                = Flag(0x59, 5);

////// CHESTS /////////////////////////////////////////////////////////////////// 

constexpr uint8_t CHEST_RYUMA_EXTERIOR = 0;
constexpr uint8_t CHEST_RYUMA_PIER = 1;
constexpr uint8_t CHEST_RYUMA_MAYOR_HOUSE = 2; // TODO
constexpr uint8_t CHEST_ROUTE_TO_RYUMA_2 = 3;
constexpr uint8_t CHEST_HELGAS_SWAMP_1 = 4; // TODO
constexpr uint8_t CHEST_HELGAS_SWAMP_2 = 5; // TODO
constexpr uint8_t CHEST_HELGAS_HUT_EXTERIOR = 6;
constexpr uint8_t CHEST_MERCATOR_MENAGERIE = 7;
constexpr uint8_t CHEST_MERCATOR_MADAME_YARD_COUNTER = 8;
constexpr uint8_t CHEST_MERCATOR_MADAME_YARD_ROOM_1 = 9;
constexpr uint8_t CHEST_MERCATOR_MADAME_YARD_ROOM_2 = 10; // TODO
constexpr uint8_t CHEST_MERCATOR_CASTLE_BIG_LIBRARY = 11;
constexpr uint8_t CHEST_MERCATOR_CASTLE_SMALL_LIBRARY_1 = 12;
constexpr uint8_t CHEST_MERCATOR_CASTLE_SMALL_LIBRARY_2 = 13;
constexpr uint8_t CHEST_MERCATOR_CASTLE_DEXTER_ROOM = 14;
constexpr uint8_t CHEST_MERCATOR_CASTLE_1F_HALLWAY = 15;

// TODO: Einstein whistle allows passing through
// TODO: CHEST_HELGAS_SWAMP_2 already open?
