#include "patch_add_golas_heart.hpp"

#include <landstalker-lib/model/map.hpp>
#include <landstalker-lib/model/world.hpp>
#include <landstalker-lib/model/entity.hpp>
#include <landstalker-lib/model/item.hpp>
#include <landstalker-lib/tools/game_text.hpp>
#include <landstalker-lib/constants/entity_type_codes.hpp>
#include <landstalker-lib/constants/map_codes.hpp>
#include <landstalker-lib/constants/item_codes.hpp>

#include "../../assets/secret_arg_music_bank.bin.hxx"
#include "../../constants/rando_constants.hpp"
#include "../../logic_model/randomizer_world.hpp"

constexpr uint8_t MUSIC_GOLAS_HEART = 20;
constexpr uint8_t MUSIC_FINAL_FIGHT = 21;

// =========================== WORLD EDITING ===========================

static MapPalette* build_golas_heart_palette(World& world)
{
    MapPalette* pal = new MapPalette(*world.map(MAP_LAKE_SHRINE_SAVE_ROOM)->palette());
    pal->at(1) = Color(0x80, 0x20, 0x20);
    pal->at(2) = Color(0xA0, 0x40, 0x20);
    pal->at(3) = Color(0xC0, 0x60, 0x40);
    pal->at(5) = Color(0x60, 0x20, 0x20);
    pal->at(6) = Color(0x60, 0x40, 0x40);
    pal->at(7) = Color(0xA0, 0x60, 0x60);
    pal->at(8) = Color(0xA0, 0x60, 0x20);
    pal->at(9) = Color(0x80, 0x00, 0x00);
    pal->at(10) = Color(0xE0, 0x40, 0x40);
    pal->at(11) = Color(0xE0, 0x80, 0x80);
    world.add_map_palette(pal);
    return pal;
}

static MapPalette* build_greyscale_palette(World& world, MapPalette* palette, float luminance = 1.f)
{
    MapPalette* pal = new MapPalette(*palette);
    for(size_t i=0 ; i<pal->size() ; ++i)
    {
        uint8_t average = (pal->at(i).r() + pal->at(i).g() + pal->at(i).b()) / 3;
        average = (uint8_t)std::min((float)average * luminance, 255.f);
        (*pal)[i] = { average, average, average };
    }

//    Color color_red = Color(0xC0, 0x00, 0x00);
//
//    MapPalette* pal = new MapPalette({
//            color_red, color_red, color_red, color_red, color_red, color_red, color_red,
//            color_red, color_red, color_red, color_red, color_red, color_red
//    });

    world.add_map_palette(pal);
    return pal;
}

static MapPalette* build_transition_palette(World& world, MapPalette* gh_pal, MapPalette* final_fight_palette)
{
    MapPalette* pal = new MapPalette();
    for(uint8_t i=0 ; i<13 ; ++i)
    {
        pal->at(i) = Color(
                (gh_pal->at(i).r() + final_fight_palette->at(i).r()) / 2,
                (gh_pal->at(i).g() + final_fight_palette->at(i).b()) / 2,
                (gh_pal->at(i).b() + final_fight_palette->at(i).b()) / 2
        );
    }
    world.add_map_palette(pal);
    return pal;
}

static MapPalette* build_golas_heart_exterior_palette(World& world)
{
    MapPalette* pal = new MapPalette(*world.map(MAP_LAKE_SHRINE_EXTERIOR_WITH_MAGMA)->palette());
    pal->at(0) = Color(0x80, 0x00, 0x00); // 008
    pal->at(3) = Color(0x40, 0x00, 0x20); // 204
    pal->at(4) = Color(0x80, 0x40, 0x00); // 048
    pal->at(5) = Color(0xC0, 0x40, 0x00); // 04C
    pal->at(8) = Color(0xE0, 0x40, 0x40); // 44E
    pal->at(9) = Color(0x60, 0x00, 0x00); // 006
    pal->at(10) = Color(0x80, 0x20, 0x20); // 228
    pal->at(11) = Color(0x80, 0x40, 0x40); // 448
    pal->at(12) = Color(0x80, 0x60, 0x60); // 668
    world.add_map_palette(pal);
    return pal;
}

static Map* add_gh_room_1(RandomizerWorld& world)
{
    Map* map = world.map(MAP_GOLAS_HEART_1);

    map->layout(world.map(MAP_LAKE_SHRINE_EXTERIOR_WITH_MAGMA)->layout());
    map->blockset(world.map(MAP_LAKE_SHRINE_EXTERIOR_WITH_MAGMA)->blockset());
    map->palette(build_golas_heart_exterior_palette(world));
    map->background_music(MUSIC_GOLAS_HEART);

    map->add_entity(new Entity({
        .type_id = ENTITY_ROCK,
        .position = Position(0x2C, 0x14, 0x6),
        .palette = 0
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_ROCK,
        .position = Position(0x2B, 0x16, 0x6),
        .palette = 0
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_ROCK,
        .position = Position(0x2B, 0x15, 0x8, true),
        .palette = 0
    }));

    return map;
}

static Map* add_gh_room_2(RandomizerWorld& world, MapPalette* pal)
{
    Map* map = world.map(MAP_GOLAS_HEART_2);

    map->layout(world.map(MAP_LAKE_SHRINE_338)->layout());
    map->blockset(world.map(MAP_LAKE_SHRINE_338)->blockset());
    map->palette(pal);
    map->background_music(MUSIC_GOLAS_HEART);

    Entity* foxy_npc = new Entity({
        .type_id = ENTITY_NPC_MAGIC_FOX,
        .position = Position(0x19, 0x1C, 0x2, true),
        .orientation = ENTITY_ORIENTATION_SW,
        .palette = 1
    });
    map->add_entity(foxy_npc);
    world.add_custom_dialogue_raw(foxy_npc,
        "Foxy: At long last, you've come!\nYou are inside Gola's heart,\nthe source of all its power.\x1e\n"
        "What you are about to face in\nhere is incredibly powerful,\nbut I have faith in you.\x1e\n"
        "Good luck. You will need it.\x03");

    return map;
}

static Map* add_gh_room_3(RandomizerWorld& world, MapPalette* pal)
{
    Map* map = world.map(MAP_GOLAS_HEART_3);

    map->layout(world.map(MAP_LAKE_SHRINE_346)->layout());
    map->blockset(world.map(MAP_LAKE_SHRINE_346)->blockset());
    map->palette(pal);
    map->background_music(MUSIC_GOLAS_HEART);
    map->global_entity_mask_flags().emplace_back(GlobalEntityMaskFlag(FLAG_CLEARED_GH_3, 0));

    map->add_entity(new Entity({
        .type_id = ENTITY_GATE_NORTH,
        .position = Position(0x19, 0x11, 0x2),
        .palette = 0,
        .behavior_id = 106
    }));
    map->add_entity(new Entity({ .type_id = ENTITY_INVISIBLE_CUBE, .position = Position(0x19, 0x12, 0x5, true) }));

    map->add_entity(new Entity({
        .type_id = ENTITY_GATE_SOUTH,
        .position = Position(0x19, 0x21, 0x2),
        .palette = 0,
        .appear_after_player_moved_away = true,
        .behavior_id = 106
    }));

    map->add_entity(new Entity({
        .type_id = ENEMY_SPINNER_2,
        .position = Position(0x15, 0x19, 0x8),
        .orientation = ENTITY_ORIENTATION_SE,
        .palette = 1
    }));

    map->add_entity(new Entity({
        .type_id = ENEMY_SPINNER_2,
        .position = Position(0x1E, 0x19, 0x8),
        .orientation = ENTITY_ORIENTATION_NW,
        .palette = 1
    }));

    return map;
}

static Map* add_gh_room_4(RandomizerWorld& world, MapPalette* pal)
{
    Map* map = world.map(MAP_GOLAS_HEART_4);

    map->layout(world.map(MAP_LAKE_SHRINE_328)->layout());
    map->blockset(world.map(MAP_LAKE_SHRINE_328)->blockset());
    map->palette(pal);
    map->background_music(MUSIC_GOLAS_HEART);
    map->visited_flag(FLAG_CLEARED_GH_3);
    map->global_entity_mask_flags().emplace_back(GlobalEntityMaskFlag(FLAG_CLEARED_GH_4, 0));

    map->add_entity(new Entity({
        .type_id = ENEMY_LIZARD_3,
        .position = Position(0x17, 0x13, 0x08),
        .palette = 0
    }));

    map->add_entity(new Entity({
        .type_id = ENEMY_LIZARD_3,
        .position = Position(0x1F, 0x13, 0x08),
        .palette = 0
    }));

    map->add_entity(new Entity({
        .type_id = ENEMY_LIZARD_3,
        .position = Position(0x13, 0x17, 0x08),
        .palette = 0
    }));

    map->add_entity(new Entity({
        .type_id = ENEMY_LIZARD_3,
        .position = Position(0x13, 0x1F, 0x08),
        .palette = 0
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_GATE_SOUTH,
        .position = Position(0x19, 0x21, 0x0),
        .palette = 0,
        .appear_after_player_moved_away = true,
        .behavior_id = 106
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_GATE_SOUTH,
        .position = Position(0x21, 0x19, 0x0),
        .orientation = ENTITY_ORIENTATION_SE,
        .palette = 0,
        .behavior_id = 106
    }));
    map->add_entity(new Entity({ .type_id = ENTITY_INVISIBLE_CUBE, .position = Position(0x20, 0x19, 0x4, true, true) }));

    return map;
}

static Map* add_gh_room_5(RandomizerWorld& world, MapPalette* pal)
{
    Map* map = world.map(MAP_GOLAS_HEART_5);

    map->layout(world.map(MAP_LAKE_SHRINE_311)->layout());
    map->blockset(world.map(MAP_LAKE_SHRINE_311)->blockset());
    map->palette(pal);
    map->background_music(MUSIC_GOLAS_HEART);
    map->visited_flag(FLAG_CLEARED_GH_4);
    map->global_entity_mask_flags().emplace_back(GlobalEntityMaskFlag(FLAG_CLEARED_GH_5, 1));

    map->add_entity(new Entity({
        .type_id = ENTITY_BUTTON,
        .position = Position(0x20, 0x13, 0x2),
        .palette = 1,
        .behavior_id = 211
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_GATE_SOUTH,
        .position = Position(0x19, 0x21, 0x2),
        .palette = 0,
        .behavior_id = 896
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_GATE_NORTH,
        .position = Position(0x11, 0x19, 0x2),
        .orientation = ENTITY_ORIENTATION_SE,
        .palette = 0,
        .behavior_id = 106
    }));

    map->add_entity(new Entity({
        .type_id = ENEMY_UNICORN_3,
        .position = Position(0x19, 0x19, 0x3, true, true),
        .palette = 0
   }));

    return map;
}

static Map* add_gh_room_6(RandomizerWorld& world, MapPalette* pal)
{
    Map* map = world.map(MAP_GOLAS_HEART_6);

    map->layout(world.map(MAP_LAKE_SHRINE_354)->layout());
    map->blockset(world.map(MAP_LAKE_SHRINE_354)->blockset());
    map->palette(pal);
    map->background_music(MUSIC_GOLAS_HEART);
    map->visited_flag(FLAG_CLEARED_GH_5);
    map->global_entity_mask_flags().emplace_back(GlobalEntityMaskFlag(FLAG_CLEARED_GH_6, 0));

    map->add_entity(new Entity({
        .type_id = ENEMY_QUAKE_2,
        .position = Position(0x19, 0x1A, 0x2),
        .orientation = ENTITY_ORIENTATION_NE,
        .palette = 1
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_GATE_NORTH,
        .position = Position(0x11, 0x19, 0x2),
        .orientation = ENTITY_ORIENTATION_SE,
        .palette = 0
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_GATE_NORTH,
        .position = Position(0x19, 0x11, 0x2),
        .palette = 0
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_GATE_SOUTH,
        .position = Position(0x21, 0x19, 0x2),
        .orientation = ENTITY_ORIENTATION_SE,
        .palette = 0,
        .behavior_id = 106
    }));

    return map;
}

static Map* add_gh_room_7(RandomizerWorld& world, MapPalette* pal)
{
    Map* map = world.map(MAP_GOLAS_HEART_7);

    map->layout(world.map(MAP_LAKE_SHRINE_342)->layout());
    map->blockset(world.map(MAP_LAKE_SHRINE_342)->blockset());
    map->palette(pal);
    map->visited_flag(FLAG_CLEARED_GH_6);
    map->background_music(15);

    Entity* nole_npc = new Entity({
        .type_id = ENEMY_NOLE,
        .position = Position(0x19, 0x13, 0x4, true, true, false),
        .orientation = ENTITY_ORIENTATION_SW,
        .palette = 0
    });
    nole_npc->fightable(false);
    nole_npc->behavior_id(0);
    map->add_entity(nole_npc);

    Entity* foxy_npc = new Entity({
        .type_id = ENTITY_NPC_MAGIC_FOX,
        .position = Position(0x13, 0x1F, 0x2),
        .orientation = ENTITY_ORIENTATION_SE,
        .palette = 3
    });
    map->add_entity(foxy_npc);
    world.add_custom_dialogue_raw(foxy_npc,
        "Foxy: The final challenge is very\nclose, you should take some rest.\x1e\n"
        "You are about to fight the very\nsource of Gola's power, what made\nits reign of terror possible.\x1e\n"
        "Please set this island free,\nonce and for all...\x03");

    map->add_entity(new Entity({
        .type_id = 0xC0 + ITEM_EKEEKE,
        .position = Position(0x15, 0x13, 0x3),
        .palette = 1,
        .liftable = true
    }));

    map->add_entity(new Entity({
        .type_id = 0xC0 + ITEM_EKEEKE,
        .position = Position(0x1E, 0x13, 0x3),
        .palette = 1,
        .liftable = true
    }));

    map->add_entity(new Entity({
        .type_id = 0xC0 + ITEM_DAHL,
        .position = Position(0x15, 0x14, 0x3),
        .palette = 1,
        .liftable = true
    }));

    map->add_entity(new Entity({
        .type_id = 0xC0 + ITEM_DAHL,
        .position = Position(0x1E, 0x14, 0x3),
        .palette = 1,
        .liftable = true
    }));

    return map;
}

static Map* add_gh_room_8(RandomizerWorld& world, MapPalette* pal)
{
    Map* map = new Map(*world.map(MAP_WATERFALL_SHRINE_181));
    map->clear_entities();
    map->id(MAP_GOLAS_HEART_8);
    map->palette(pal);
    map->background_music(MUSIC_GOLAS_HEART);

    map->add_entity(new Entity({
        .type_id = ENEMY_ORC_3,
        .position = Position(21, 21, 1),
        .palette = 0
    }));

    map->add_entity(new Entity({
        .type_id = ENEMY_ORC_3,
        .position = Position(21, 16, 1),
        .palette = 0
    }));

    map->add_entity(new Entity({
        .type_id = ENEMY_ORC_3,
        .position = Position(16, 16, 1),
        .palette = 0
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_GATE_SOUTH,
        .position = Position(0x1A, 0x12, 0x1),
        .orientation = ENTITY_ORIENTATION_SE,
        .palette = 0,
        .appear_after_player_moved_away = true,
        .behavior_id = 785
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_SMALL_THIN_DARK_PLATFORM,
        .position = Position(0x12, 0x13, 0x0, false, false, true),
        .palette = 0,
        .behavior_id = 106
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_SMALL_THIN_DARK_PLATFORM,
        .position = Position(0x13, 0x13, 0x0, false, false, true),
        .palette = 0,
        .behavior_id = 106
    }));

    world.add_map(map);
    return map;
}

static Map* add_gh_room_9(RandomizerWorld& world, MapPalette* pal)
{
    Map* map = world.map(MAP_GOLAS_HEART_9);

    map->layout(world.map(MAP_WATERFALL_SHRINE_175)->layout());
    map->blockset(world.map(MAP_WATERFALL_SHRINE_175)->blockset());
    map->palette(pal);
    map->background_music(MUSIC_GOLAS_HEART);

    map->add_entity(new Entity({
        .type_id = ENTITY_GATE_NORTH,
        .position = Position(0x1F, 0x11, 0x7),
        .orientation = ENTITY_ORIENTATION_SW,
        .palette = 0
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_LARGE_GRAY_SPIKEBALL,
        .position = Position(0x1B, 0x21, 0x1, false, false, true),
        .palette = 0,
        .speed = 3,
        .fightable = true,
        .gravity_immune = true,
        .behavior_id = 371
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_LARGE_GRAY_SPIKEBALL,
        .position = Position(0x1F, 0x21, 0x1, false, false, true),
        .palette = 0,
        .speed = 3,
        .fightable = true,
        .gravity_immune = true,
        .behavior_id = 371
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_LARGE_GRAY_SPIKEBALL,
        .position = Position(0x1D, 0x30, 0x1),
        .palette = 0,
        .speed = 3,
        .fightable = true,
        .gravity_immune = true,
        .behavior_id = 371
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_LARGE_GRAY_SPIKEBALL,
        .position = Position(0x1C, 0x33, 0x2),
        .palette = 0,
        .speed = 4,
        .fightable = true,
        .gravity_immune = true,
        .behavior_id = 74
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_LARGE_GRAY_SPIKEBALL,
        .position = Position(0x16, 0x33, 0x2, false, false, true),
        .palette = 0,
        .speed = 4,
        .fightable = true,
        .gravity_immune = true,
        .behavior_id = 74
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_LARGE_GRAY_SPIKEBALL,
        .position = Position(0x10, 0x33, 0x2),
        .palette = 0,
        .speed = 4,
        .fightable = true,
        .gravity_immune = true,
        .behavior_id = 74
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_LARGE_GRAY_SPIKEBALL,
        .position = Position(0x1F, 0x25, 0x2),
        .palette = 0,
        .speed = 4,
        .fightable = true,
        .gravity_immune = true,
        .behavior_id = 74
    }));

    return map;
}

static Map* add_gh_stairs_room(RandomizerWorld& world, MapPalette* pal, uint16_t map_id)
{
    Map* map = new Map(*world.map(MAP_THIEVES_HIDEOUT_218));
    map->clear_entities();
    map->id(map_id);
    map->palette(pal);
    map->background_music(18);

    world.add_map(map);
    return map;
}

static Map* add_gh_falling_room(RandomizerWorld& world, MapPalette* pal)
{
    Map* map = new Map(*world.map(MAP_LAKE_SHRINE_320));
    map->clear_entities();
    map->id(MAP_GOLAS_HEART_FALLING_ROOM);
    map->palette(pal);
    map->background_music(18);

    map->add_entity(new Entity({
        .type_id = ENTITY_GATE_NORTH,
        .position = Position(0x11, 0x19, 0x2),
        .orientation = ENTITY_ORIENTATION_SE,
        .palette = 0
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_GATE_NORTH,
        .position = Position(0x19, 0x11, 0x2),
        .palette = 0
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_LARGE_DARK_PLATFORM,
        .position = Position(0x19, 0x19, 0x0, false, false, true),
        .palette = 0,
        .behavior_id = 70
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_GODDESS_STATUE,
        .position = Position(0x16, 0x19, 0x2, false, true, false),
        .orientation = ENTITY_ORIENTATION_SE,
        .palette = 0
    }));

    world.add_map(map);
    return map;
}

static Map* add_gh_final_fight_room(RandomizerWorld& world, MapPalette* pal)
{
    Map* map = new Map(*world.map(MAP_NOLE_ARENA));
    map->clear_entities();
    map->id(MAP_GOLAS_HEART_DARK_NOLE_FIGHT);
    map->palette(pal);
    map->background_music(MUSIC_FINAL_FIGHT);

    map->add_entity(new Entity({
        .type_id = ENEMY_NOLE,
        .position = Position(0x1D, 0x16, 0x4),
        .orientation = ENTITY_ORIENTATION_SW,
        .palette = 1
    }));

    map->add_entity(new Entity({
        .type_id = ENTITY_SMALL_GRAY_SPIKEBALL,
        .position = Position(0x1A, 0x1E, 0x4),
        .palette = 1,
        .speed = 4,
        .fightable = true,
        .gravity_immune = true,
        .behavior_id = 371
    }));

    map->add_entity(new Entity({ .type_id = ENTITY_INVISIBLE_CUBE, .position=Position(0x1D, 0x22, 0x5, false, false, true) }));
    map->add_entity(new Entity({ .type_id = ENTITY_INVISIBLE_CUBE, .position=Position(0x1E, 0x22, 0x5, false, false, true) }));
    map->add_entity(new Entity({ .type_id = ENTITY_INVISIBLE_CUBE, .position=Position(0x1F, 0x22, 0x5, false, false, true) }));

    world.add_map(map);
    return map;
}

void PatchAddGolasHeart::alter_world(World& w)
{
    RandomizerWorld& world = reinterpret_cast<RandomizerWorld&>(w);

    MapPalette* golas_heart_palette = build_golas_heart_palette(world);
    MapPalette* stairs_palette_2 = build_greyscale_palette(world, world.map(MAP_LAKE_SHRINE_342)->palette());
    MapPalette* stairs_palette_1 = build_transition_palette(world, golas_heart_palette, stairs_palette_2);
    MapPalette* final_fight_palette = build_greyscale_palette(world, world.map(MAP_NOLE_ARENA)->palette(), 0.4);

    _room_1 = add_gh_room_1(world);

    // Archway
    _room_2 = add_gh_room_2(world, golas_heart_palette);
    world.map_connections().emplace_back(MapConnection(_room_1->id(), 33, 48, _room_2->id(), 25, 34, 2));

    // Double red spinner room
    _room_3 = add_gh_room_3(world, golas_heart_palette);
    world.map_connections().emplace_back(MapConnection(_room_2->id(), 25, 23, _room_3->id(), 25, 34, 2));

    // Lizards room
    _room_4 = add_gh_room_4(world, golas_heart_palette);
    world.map_connections().emplace_back(MapConnection(_room_3->id(), 25, 19, _room_4->id(), 25, 34, 2));

    // Solo unicorn room
    _room_5 = add_gh_room_5(world, golas_heart_palette);
    world.map_connections().emplace_back(MapConnection(_room_5->id(), 19, 25, _room_4->id(), 34, 25, 4));

    // Mirrors T-room
    _room_6 = add_gh_room_6(world, golas_heart_palette);
    world.map_connections().emplace_back(MapConnection(_room_6->id(), 25, 19, _room_5->id(), 25, 34, 2));
    world.map_connections().emplace_back(MapConnection(_room_6->id(), 19, 25, _room_2->id(), 34, 25, 4));

    // Healing room
    _room_7 = add_gh_room_7(world, golas_heart_palette);
    world.map_connections().emplace_back(MapConnection(_room_7->id(), 19, 25, _room_6->id(), 34, 25, 4));

    // WS orc arena
    _room_8 = add_gh_room_8(world, golas_heart_palette);
    world.map_connections().emplace_back(MapConnection(_room_2->id(), 19, 25, _room_8->id(), 27, 18, 4));

    // WS underground
    _room_9 = add_gh_room_9(world, golas_heart_palette);
    world.map_connections().emplace_back(MapConnection(_room_9->id(), 31, 18, _room_8->id(), 18, 19, 10));

    // Giant stairs
    _room_10 = add_gh_stairs_room(world, golas_heart_palette, MAP_GOLAS_HEART_STAIRS_1);
    world.map_connections().emplace_back(MapConnection(_room_9->id(), 16, 51, _room_10->id(), 45, 22, 4));
    _room_11 = add_gh_stairs_room(world, stairs_palette_1, MAP_GOLAS_HEART_STAIRS_2);
    world.map_connections().emplace_back(MapConnection(_room_10->id(), 20, 22, _room_11->id(), 45, 22, 4));
    _room_12 = add_gh_stairs_room(world, stairs_palette_2, MAP_GOLAS_HEART_STAIRS_3);
    world.map_connections().emplace_back(MapConnection(_room_11->id(), 20, 22, _room_12->id(), 45, 22, 4));

    // Falling room
    _room_13 = add_gh_falling_room(world, stairs_palette_2);
    world.map_connections().emplace_back(MapConnection(_room_12->id(), 20, 22, _room_13->id(), 34, 25, 4));

    // Dark Nole room
    _room_14 = add_gh_final_fight_room(world, final_fight_palette);
    _room_13->fall_destination(_room_14->id());
}


// =========================== MUSIC HANDLING ===========================

void PatchAddGolasHeart::inject_data(md::ROM& rom, World& world)
{
    rom.set_bytes(0x1F8000, ARG_MUSIC_BANK, ARG_MUSIC_BANK_SIZE);
    rom.set_word(0x1F61ED, 0x328A); // Change music indexes in the sound driver

    std::vector<uint8_t> room_music_lut = rom.get_bytes(0x2A32, 0x2A46);
    room_music_lut.emplace_back(0x1F); // Gola's heart theme
    room_music_lut.emplace_back(0x04); // Final boss theme
    _new_room_music_lut_addr = rom.inject_bytes(room_music_lut);
}

static void adapt_room_music_lookup(md::ROM& rom, uint32_t room_music_lut_addr)
{
    md::Code func;
    func.movem_to_stack({}, { reg_A0 });
    {
        func.extw(reg_D0);
        func.lea(room_music_lut_addr, reg_A0);
        func.moveb(addrw_(reg_A0, reg_D0), reg_D0);
    }
    func.movem_from_stack({}, { reg_A0 });
    func.rts();

    uint32_t addr = rom.inject_code(func);
    rom.set_code(0x29F8, md::Code().jsr(addr));
}


// =========================== CODE INJECTION ===========================

static void add_func_reinforce_room_4_lizards(md::ROM& rom, Map* map)
{
    md::Code func_init_map;
    func_init_map.movem_to_stack({ reg_D0 }, { reg_A0 });
    {
        func_init_map.movel(0x4, reg_D0);
        func_init_map.lea(0xFF5480, reg_A0);
        func_init_map.label("loop_entities");
        {
            func_init_map.moveb(0x38, addr_(reg_A0, 0x3C));
            func_init_map.moveb(0x30, addr_(reg_A0, 0x3E));
            func_init_map.adda(0x80, reg_A0);
        }
        func_init_map.dbra(reg_D0, "loop_entities");
    }
    func_init_map.movem_from_stack({ reg_D0 }, { reg_A0 });
    func_init_map.rts();

    map->map_setup_addr(rom.inject_code(func_init_map));
}

static void add_func_reinforce_room_5_unicorn(md::ROM& rom, Map* map)
{
    md::Code func_init_map;
    {
        // Empower the unicorn
        func_init_map.moveb(0x15, addr_(0xFF563C)); // 07 dmg
        func_init_map.moveb(0x50, addr_(0xFF563E)); // 17 health
    }
    func_init_map.rts();

    map->map_setup_addr(rom.inject_code(func_init_map));
}

static void add_func_reinforce_room_6_quake(md::ROM& rom, Map* map)
{
    md::Code func_init_map;
    {
        // Empower the quake
        func_init_map.moveb(0x80, addr_(0xFF54BC)); // 0C dmg
        func_init_map.moveb(0x0F, addr_(0xFF54FC)); // 07 armor
    }
    func_init_map.rts();

    map->map_setup_addr(rom.inject_code(func_init_map));
}

static void add_func_reinforce_room_8_orcs(md::ROM& rom, Map* map)
{
    md::Code func_init_map;
    func_init_map.movem_to_stack({ reg_D0 }, { reg_A0 });
    {
        func_init_map.movel(0x3, reg_D0);
        func_init_map.lea(0xFF5480, reg_A0);
        func_init_map.label("loop_entities");
        {
            func_init_map.moveb(0x50, addr_(reg_A0, 0x3C));
            func_init_map.moveb(0xA0, addr_(reg_A0, 0x3E));
            func_init_map.adda(0x80, reg_A0);
        }
        func_init_map.dbra(reg_D0, "loop_entities");
    }
    func_init_map.movem_from_stack({ reg_D0 }, { reg_A0 });
    func_init_map.rts();

    map->map_setup_addr(rom.inject_code(func_init_map));
}

static void add_func_reinforce_room_9_spikeballs(md::ROM& rom, Map* map)
{
    md::Code func_init_map;
    func_init_map.movem_to_stack({ reg_D0 }, { reg_A0 });
    {
        func_init_map.movel(map->entities().size()-1, reg_D0);
        func_init_map.lea(0xFF5500, reg_A0);
        func_init_map.label("loop_entities");
        {
            // Strengthen boulders
            func_init_map.moveb(0xA0, addr_(reg_A0, 0x3C)); // One shot
            func_init_map.adda(0x80, reg_A0);
        }
        func_init_map.dbra(reg_D0, "loop_entities");
    }
    func_init_map.movem_from_stack({ reg_D0 }, { reg_A0 });
    func_init_map.rts();

    map->map_setup_addr(rom.inject_code(func_init_map));
}

static void add_func_reinforce_dark_nole(md::ROM& rom, Map* map)
{
    md::Code func_init_map;
    func_init_map.movem_to_stack({ reg_D0 }, {});
    {
        func_init_map.moveb(0, addr_(0xFF001C));

        // Make Nigel spawn higher
        func_init_map.movew(0xE0, addr_(0xFF5412));

        func_init_map.moveb(0x60, addr_(0xFF54BC)); // Increase Nole damage
        func_init_map.movew(0x4000, addr_(0xFF54BE)); // Increase Nole HP
        func_init_map.movew(0x9000, addr_(0xFF54FC)); // Increase Nole armor

        func_init_map.moveb(0x15, addr_(0xFF553C)); // Make the spikeball a bit more damaging
    }
    func_init_map.movem_from_stack({ reg_D0 }, {});
    func_init_map.rts();

    map->map_setup_addr(rom.inject_code(func_init_map));
}

static uint32_t inject_end_cutscene(md::ROM& rom, World& world)
{
    md::Code func;

    // Notify Archipelago client we completed the game
    func.moveb(0x01, addr_(ADDR_ARCHIPELAGO_COMPLETION_BYTE));

    // Cut the music and wait
    func.trap(0, { 0x00, 0xFD });
    func.jsr(0x29046); // Sleep
    func.add_word(0x0040);

    // Turn Nigel black
    for(uint32_t offset = 0x40 ; offset < 0x60 ; offset += 0x2)
    {
        func.movel(0xC0000000 + (offset * 0x10000), addr_(0xC00004)); // Write CRAM
        func.movew(0, addr_(0xC00000));
    }

    // Turn the screen to red
    func.movew(0x0006, reg_D0);
    func.clrl(reg_D1);
    func.movew(0x4, reg_D1);
    func.label("loop_turn_screen_red");
    {
        for(uint32_t offset = 0x00 ; offset < 0x20 ; offset += 0x2)
        {
            func.movel(0xC0000000 + (offset * 0x10000), addr_(0xC00004)); // Write CRAM
            func.movew(reg_D0, addr_(0xC00000));
        }
        for(uint32_t offset = 0x60 ; offset < 0x80 ; offset += 0x2)
        {
            func.movel(0xC0000000 + (offset * 0x10000), addr_(0xC00004)); // Write CRAM
            func.movew(reg_D0, addr_(0xC00000));
        }
        func.addib(2, reg_D0);
        func.jsr(0x29046); // Sleep_0 for 0x20 frames
        func.add_word(0x0020);
    }
    func.dbra(reg_D1, "loop_turn_screen_red");

    func.jsr(0x29046); // Sleep_0 for 120 frames
    func.add_word(120);

    // Make a dramatic looping death sound
    func.movew(16, reg_D1);
    func.label("loop_gola_yell");
    {
        func.trap(0, { 0x00, 0x72 });
        func.jsr(0x29046); // Sleep_0 for 15 frames
        func.add_word(12);
    }
    func.dbra(reg_D1, "loop_gola_yell");

    func.jsr(0x29046); // Sleep_0 for 120 frames
    func.add_word(60);

    // Turn Nigel to red
    func.movew(0x0002, reg_D0);
    func.clrl(reg_D1);
    func.movew(0x6, reg_D1);
    func.label("loop_turn_nigel_red");
    {
        for(uint32_t offset = 0x40 ; offset < 0x60 ; offset += 0x2)
        {
            func.movel(0xC0000000 + (offset * 0x10000), addr_(0xC00004)); // Write CRAM
            func.movew(reg_D0, addr_(0xC00000));
        }
        func.addib(2, reg_D0);
        func.jsr(0x29046); // Sleep_0 for 4 frames
        func.add_word(4);
    }
    func.dbra(reg_D1, "loop_turn_nigel_red");

    func.jsr(0x29046); // Sleep_0 for 120 frames
    func.add_word(60);

    // Turn the screen to black
    func.movew(0x000A, reg_D0);
    func.clrl(reg_D1);
    func.movew(0x5, reg_D1);
    func.label("loop_turn_screen_black");
    {
        for(uint32_t offset = 0x00 ; offset < 0x20 ; offset += 0x2)
        {
            func.movel(0xC0000000 + (offset * 0x10000), addr_(0xC00004)); // Write CRAM
            func.movew(reg_D0, addr_(0xC00000));
        }
        for(uint32_t offset = 0x40 ; offset < 0x60 ; offset += 0x2)
        {
            func.movel(0xC0000000 + (offset * 0x10000), addr_(0xC00004)); // Write CRAM
            func.movew(reg_D0, addr_(0xC00000));
        }
        for(uint32_t offset = 0x60 ; offset < 0x80 ; offset += 0x2)
        {
            func.movel(0xC0000000 + (offset * 0x10000), addr_(0xC00004)); // Write CRAM
            func.movew(reg_D0, addr_(0xC00000));
        }
        func.subib(2, reg_D0);
        func.jsr(0x29046); // Sleep_0 for 8 frames
        func.add_word(8);
    }
    func.dbra(reg_D1, "loop_turn_screen_black");

    func.jsr(0x29046); // Sleep_0 for 3s
    func.add_word(60*3);
    func.trap(0, { 0x00, 0x2E }); // Play spell book music
    func.jsr(0x29046); // Sleep_0 for 3s
    func.add_word(60*3);

    // Mask HUD
    func.lea(0xFF1C00, reg_A1);
    func.movew(0x07FF, reg_D7);
    func.label("loop_erase_hud");
    {
        func.movew(0xE6B4, addr_(reg_A1));
        func.adda(2, reg_A1);
    }
    func.dbra(reg_D7, "loop_erase_hud");
    func.jsr(0x79E4); // QueueFullHUDTilemapDMA

    // Put back white in the last palette for textboxes
    func.movel(0xC0000000 + (0x76 * 0x10000), addr_(0xC00004)); // Write CRAM
    func.movew(0xEEE, addr_(0xC00000));

    // ----------------------------------------------------------

    std::vector<std::string> textboxes = {
            "This is where the tales of\nNigel the treasure hunter end.\n ",
            "\nOnce he freed the island from\nGola the ancient dragon god,\nnobody saw him ever again.",
            "\nSome people pretend he died\nwhile dealing the last blow\nbanishing Gola for eternity.",
            "\nSome other pretend he sailed back\nto the continent, chasing the\ngreat Heart of Diamond.",
            "\nThe truth is, no one knows,\nbut the island has always lived\nin peace ever since.",
            "\nThe bears native of Gumi\ncelebrate this day every year,\nkeeping the legend alive.",
            "\nAs for myself, I'm glad I met\nhim during his grand journey.\n                        \x1a\x1a\x1a\x1aPOCKETS\x1a\x1a\x1a\x1a\x1a\x1a\x1a"
    };

    ByteArray end_textboxes;
    for(size_t i=0 ; i<textboxes.size() ; ++i)
    {
        uint16_t text_id = world.first_empty_game_string_id(0x4D);
        world.game_strings()[text_id] = textboxes[i] + "\x1a\x1a\x1a\x1a\x1a\x1a";
        if(i < textboxes.size() - 1)
            end_textboxes.add_word(0x8000 + (text_id - 0x4D));
        else
            end_textboxes.add_word(0xE000 + (text_id - 0x4D));
    }
    uint32_t cutscene_addr = rom.inject_bytes(end_textboxes);

    // ----------------------------------------------------------

    func.jsr(0xBA4); // FlushDMACopyQueue
    func.lea(cutscene_addr, reg_A0);
    func.movel(0x1, reg_D0);
    func.jsr(0x253F8); // RunTextCmd

    // Cut the music and wait
    func.trap(0, { 0x00, 0xFD });
    func.jsr(0x29046); // Sleep
    func.add_word(120);

    func.jmp(0x1570C); // Launch the credits

    return rom.inject_code(func);
}

static void add_final_fight_update_handler(md::ROM& rom, Map* map, uint32_t end_cutscene_addr)
{
    constexpr uint8_t PEAK_COLOR = 0xC;
    constexpr uint8_t VULNERABILITY_THRESHOLD = 0x8;

    ByteArray palette_sequence;
    for(uint8_t byte = 0x00 ; byte <= PEAK_COLOR ; byte += 0x02)
        palette_sequence.add_byte(byte);
    for(uint8_t byte = PEAK_COLOR - 0x02 ; byte >= 0x02 ; byte -= 0x02)
        palette_sequence.add_byte(byte);
    while(palette_sequence.size() < 32)
        palette_sequence.add_byte(0);

    uint32_t palette_sequence_addr = rom.inject_bytes(palette_sequence);

    md::Code proc;
    {
        proc.movem_to_stack({}, { reg_A0 });
        {
            proc.clrl(reg_D0);
            proc.addib(1, addr_(0xFF001C));
            proc.moveb(addr_(0xFF001C), reg_D0);
            proc.andib(0x01, reg_D0);
            proc.beq("continue");
            {
                proc.movem_from_stack({}, { reg_A0 });
                proc.rts();
            }
            proc.label("continue");
            proc.moveb(addr_(0xFF001C), reg_D0);
            proc.lsrb(2, reg_D0);
            proc.andib(palette_sequence.size()-1, reg_D0);
            proc.lea(palette_sequence_addr, reg_A0);
            proc.moveb(addr_(reg_A0, reg_D0), reg_D0);
            for(uint32_t offset = 0x20 ; offset < 0x40 ; offset += 0x2)
            {
                proc.movel(0xC0000000 + (offset * 0x10000), addr_(0xC00004)); // Write CRAM
                proc.movew(reg_D0, addr_(0xC00000));
            }

            proc.cmpiw(0xFFFF, addr_(0xFF5480));
            proc.bne("not_removed");
            {
                proc.cmpib(2, addr_(0xFF001C));
                proc.bne("end");
                proc.jsr(end_cutscene_addr);
            }
            proc.label("not_removed");
            proc.cmpiw(0x0001, addr_(0xFF54BE));
            proc.bne("not_defeated");
            {
                // If Nole is defeated, remove it
                proc.movew(0xFFFF, addr_(0xFF5480));
                proc.moveb(0x00, addr_(0xFF001C));
                proc.moveb(0x7, addr_(0xFF1153)); // Prevent player from attacking & opening menu
                proc.moveb(0x1, addr_(0xFF540C)); // Freeze Nigel animation
            }
            proc.label("not_defeated");
            proc.cmpib(VULNERABILITY_THRESHOLD, reg_D0);
            proc.bge("vulnerable");
            {
                proc.moveb(0x01, addr_(0xFF54B8)); // Make Nole invicible
                proc.bra("end");
            }
            proc.label("vulnerable");
            {
                proc.moveb(0x00, addr_(0xFF54B8)); // Make Nole vulnerable
            }
        }
        proc.label("end");
        proc.movem_from_stack({}, { reg_A0 });
        proc.rts();
    }

    map->map_update_addr(rom.inject_code(proc));
}

void PatchAddGolasHeart::inject_code(md::ROM& rom, World& world)
{
    add_func_reinforce_room_4_lizards(rom, _room_4);
    add_func_reinforce_room_5_unicorn(rom, _room_5);
    add_func_reinforce_room_6_quake(rom, _room_6);
    add_func_reinforce_room_8_orcs(rom, _room_8);
    add_func_reinforce_room_9_spikeballs(rom, _room_9);

    add_func_reinforce_dark_nole(rom, _room_14);
    uint32_t end_cutscene_addr = inject_end_cutscene(rom, world);
    add_final_fight_update_handler(rom, _room_14, end_cutscene_addr);

    adapt_room_music_lookup(rom, _new_room_music_lut_addr);
}