#include "kaizo.hpp"

void setup_ryuma_maps(World& world)
{
    wipe_map_variants(world, {
        MAP_RYUMA_EXTERIOR,
        MAP_RYUMA_MIDDLE_HOUSE,
        MAP_RYUMA_LIGHTHOUSE
    });

}

void edit_ryuma_spawn_house(World& world)
{
    world.spawn_location(SpawnLocation("kaizo", MAP_RYUMA_MIDDLE_HOUSE, 16, 16, ENTITY_ORIENTATION_SE, 10));

    Map* map = world.map(MAP_RYUMA_MIDDLE_HOUSE);
    map->clear_entities();
}

void edit_ryuma_exterior(World& world)
{
    Map* map = world.map(MAP_RYUMA_EXTERIOR);
    wipe_map_variants(map);

    map->add_entity(new Entity({ 
        .type_id = ENTITY_CHEST,
        .position = Position(0x2F, 0x35, 1),
        .orientation = ENTITY_ORIENTATION_NW
    }));
    map->base_chest_id(0);
    ItemSource* source = new ItemSourceChest(0, "");
    source->item(world.item(ITEM_EKEEKE));
    world.item_sources().push_back(source);
}

void edit_ryuma_pier(World& world)
{
    Map* map = world.map(MAP_RYUMA_RAFT_PIER);
    map->clear_entities();


    map->add_entity(new Entity({ 
        .type_id = ENTITY_CHEST,
        .position = Position(0x27, 0x27, 1)
    }));
    map->base_chest_id(1);
    ItemSource* source = new ItemSourceChest(1, "");
    source->item(world.item(ITEM_EKEEKE));
    world.item_sources().push_back(source);

    batch_add_entities(map, 
        { Position(0x27, 0x26, 3), Position(0x27, 0x26, 5, false, false, true) }, 
        { .type_id = ENTITY_INVISIBLE_CUBE });
}

void edit_route_to_ryuma_2(World& world)
{
    Map* map = world.map(MAP_ROUTE_TO_RYUMA_2);

    Map* base_map = world.map(MAP_ROUTE_AFTER_DESTEL_1);
    map->address(base_map->address());
    map->tileset_id(base_map->tileset_id());
    map->primary_big_tileset_id(base_map->primary_big_tileset_id());
    map->secondary_big_tileset_id(base_map->secondary_big_tileset_id());

    MapConnection& connection_sw = world.map_connection(MAP_RYUMA_EXTERIOR, MAP_ROUTE_TO_RYUMA_2);
    connection_sw.pos_x_2(28);
    connection_sw.pos_y_2(62);

    MapConnection& connection_ne = world.map_connection(MAP_ROUTE_TO_RYUMA_2, MAP_ROUTE_TO_RYUMA_1);
    connection_ne.pos_x_1(33);
    connection_ne.pos_y_1(14);

    map->clear_entities();
}

//void edit_route_to_ryuma_1(World& world)
//{
//    Map* map = world.map(MAP_ROUTE_TO_RYUMA_1);
//
//    MapPalette* original_green_palette = world.map(MAP_ROUTE_TO_RYUMA_1)->palette();
//
//    MapPalette* snowy_palette = new MapPalette(*original_green_palette);
//    snowy_palette->color(3) = MapPalette::Color(0xA, 0xA, 0xC);
//    snowy_palette->color(4) = MapPalette::Color(0xC, 0xC, 0xE);
//    snowy_palette->color(5) = MapPalette::Color(0xE, 0xE, 0xE);
//    world.map_palettes().push_back(snowy_palette);
//    
//    map->palette(snowy_palette);
//}

void edit_kado_climb_map(World& world)
{
    Map* map = world.map(MAP_ROUTE_GUMI_RYUMA_KADO_CLIMB);
    Entity* chest = map->entity(8);
    chest->entity_type_id(ENTITY_CRATE); // Turn chest into a crate
    chest->liftable(true);
    map->entity(4)->position().half_y = true; // Move slightly a platform towards SW to make a jump possible
    map->entity(5)->position().half_z = true; // Move slightly a platform up to make a jump possible
    map->entity(9)->position().z = 7; // Raise a platform to serve as a place where to put the crate
    map->remove_entity(13); // Remove the spikeball
    map->remove_entity(12); // Remove the final platform
    map->remove_entity(10); // Remove the platform right before the chest
    map->remove_entity(6); // Remove the thin platform moving around
    map->remove_entity(3); // Remove an intermediate platform
    map->remove_entity(2); // Remove an intermediate platform
}

void edit_kado_textlines(World& world)
{
    world.game_strings()[0x736] = "\x1cMy name is Kado.\nI lost my Safety Pass\non the route yesterday.\x1e";
    world.game_strings()[0x737] = "\x1cI think witch Helga would\nbe able to locate it magically,\nbut I'm too old to go there.\x1e";
    world.game_strings()[0x738] = "\x1cYou can find her in the\nswamp not far from Mercator,\nnorth from here.\x1e";
    world.game_strings()[0x739] = "\x1cThis will help you against\nthe dangers of this world.";
}

void edit_helga_swamp_map(World& world)
{
    Map* map = world.map(MAP_ROUTE_GUMI_RYUMA_HELGA_SWAMP);
    map->clear_entities();
    map->add_entity(new Entity(ENTITY_SMALL_GRAY_SPIKEBALL, 0x16, 0x27, 4));
    map->add_entity(new Entity(ENTITY_SMALL_YELLOW_PLATFORM, 0x17, 0x27, 4))->position().half_x = true;

    map->add_entity(new Entity(ENTITY_SMALL_GRAY_SPIKEBALL, 0x1B, 0x27, 4));
    map->add_entity(new Entity(ENTITY_SMALL_YELLOW_PLATFORM, 0x1B, 0x25, 4))->position().half_y = true;

    map->add_entity(new Entity(ENTITY_SMALL_YELLOW_PLATFORM, 0x1B, 0x21, 4));
    map->add_entity(new Entity(ENTITY_SMALL_GRAY_SPIKEBALL, 0x1B, 0x1F, 4))->position().half_y = true;

    map->add_entity(new Entity(ENTITY_SMALL_GRAY_SPIKEBALL, 0x18, 0x1F, 4));
    map->add_entity(new Entity(ENTITY_SMALL_YELLOW_PLATFORM, 0x18, 0x1C, 4));

    map->add_entity(new Entity(ENTITY_SMALL_GRAY_SPIKEBALL, 0x19, 0x15, 5));
    auto moving_platform = map->add_entity(new Entity(ENTITY_SMALL_YELLOW_PLATFORM, 0x19, 0x1B, 4));
    moving_platform->behavior_id(451);
    moving_platform->speed(2);
    moving_platform->gravity_immune(true);

    map->add_entity(new Entity(ENTITY_LARGE_GRAY_SPIKEBALL, 0x1B, 0x11, 1))->palette(2);
    map->add_entity(new Entity(ENTITY_LARGE_THIN_YELLOW_PLATFORM, 0x1B, 0x11, 4));
    map->add_entity(new Entity(ENTITY_LARGE_THIN_YELLOW_PLATFORM, 0x1B, 0x13, 4));
    map->add_entity(new Entity(ENTITY_LARGE_THIN_YELLOW_PLATFORM, 0x19, 0x13, 4));

    map->add_entity(new Entity(ENTITY_CHEST, 0x12, 0x15, 5));
    map->base_chest_id(134); // Use the chest ID of removed Kado climb chest
}

void edit_helga_swamp_map_2(World& world)
{
    Map* map = world.map(MAP_ROUTE_GUMI_RYUMA_HELGA_SWAMP_2);
    map->entity(1)->speed(4);

    map->add_entity(new Entity(ENTITY_LARGE_THIN_YELLOW_PLATFORM, 0x22, 0x29, 4));
    map->add_entity(new Entity(ENTITY_LARGE_GRAY_SPIKEBALL, 0x22, 0x28, 1))->palette(2);
    map->add_entity(new Entity(ENTITY_LARGE_THIN_YELLOW_PLATFORM, 0x22, 0x27, 4));
    map->add_entity(new Entity(ENTITY_SMALL_YELLOW_PLATFORM, 0x25, 0x27, 2));
    map->add_entity(new Entity(ENTITY_SMALL_YELLOW_PLATFORM, 0x25, 0x25, 0))->position().half_z = true;

    map->add_entity(new Entity(ENTITY_SACRED_TREE, 0x19, 0x23, 5))->position().half_x = true;
    map->add_entity(new Entity(ENTITY_SACRED_TREE, 0x1A, 0x23, 5))->position().half_x = true;

    map->add_entity(new Entity(ENTITY_SMALL_THIN_YELLOW_PLATFORM, 0x1F, 0x1A, 4));
    map->add_entity(new Entity(ENTITY_SMALL_THIN_YELLOW_PLATFORM, 0x22, 0x1A, 6));

    map->add_entity(new Entity(ENTITY_CHEST, 0x19, 0x1A, 7))->orientation(ENTITY_ORIENTATION_NE);
    map->base_chest_id(0x00); // TODO! Use an empty chest ID

    map->remove_entity(0);
}

void fix_helga_hut_palette(World& world)
{
    std::array<uint16_t, 13> helga_palette_words = { 0x0204, 0x0426, 0x086A, 0x0222, 0x0444, 0x0888, 
                                                     0x000A, 0x04AA, 0x024A, 0x0028, 0x0066, 0x0A46, 0x0060 };

    MapPalette* palette = new MapPalette(helga_palette_words);
    world.map_palettes().push_back(palette);
    world.map(MAP_HELGAS_HUT)->palette(palette);
}

void edit_helga_dialogue(World& world, md::ROM& rom)
{
    world.game_strings()[0x497] = "\x1cYou want to find a lost\nitem... Let me take a look...\nOh... I can feel it...\x03"
                                  "\nWhat you are looking for\nhas been eaten by a blue worm.\nIt's not far away from here.\x03"
                                  "\nLook for that blue worm and\nyou might find your pass\nto Mercator city.\x03"
                                  "\nIf you go there, say hello\nto my sister Linda. She works\nthere as a fortune teller.\x03";

    world.game_strings()[0x498] = "\x1cHave you found that worm?\nI can feel it not that far away.\nBut it seems powerful...\x03";

    md::Code helga_dialogue_script;
    helga_dialogue_script.btst(4, addr_(0xFF1004));
    helga_dialogue_script.beq(3);
        helga_dialogue_script.trap(0x1, { 0x00, 0x21 }); // Show reminder cutscene
        helga_dialogue_script.rts();
    helga_dialogue_script.btst(7, addr_(0xFF101B));
    helga_dialogue_script.beq(4);
        helga_dialogue_script.bset(4, addr_(0xFF1004));
        helga_dialogue_script.trap(0x1, { 0x00, 0x0D }); // Show "locating Safety Pass" cutscene
        helga_dialogue_script.rts();
    helga_dialogue_script.trap(0x1, { 0x00, 0x09 }); // Show useless chitchat as long as Kado has not been talked to
    helga_dialogue_script.rts();

    // Reminder cutscene
    helga_dialogue_script.add_bytes({
        0xE4, 0x4B // Print message 0x498 and close the textbox
    });
    // "Locating Safety Pass" cutscene
    helga_dialogue_script.add_bytes({
        0xE4, 0x4A // Print message 0x497 and close the textbox
    });
    // Useless chitchat as long as Kado has not been talked to
    helga_dialogue_script.add_bytes({
        0xE4, 0x75 // Print message 0x4C2 and close the textbox
    });

    uint32_t addr = rom.inject_code(helga_dialogue_script);
    rom.set_code(0x2731A, md::Code().jmp(addr));
}

void add_ultra_worm_to_gumi_boulder_map(World& world)
{
    Map* map = world.map(MAP_ROUTE_GUMI_RYUMA_BOULDER);
    auto ultra_worm = map->add_entity(new Entity(ENEMY_WORM_3, 0x18, 0x1D, 1));
    ultra_worm->only_when_flag_is_set(FLAG_HELGA_TALKED_ABOUT_WORM);
    ultra_worm->remove_when_flag_is_set(FLAG_SAFETY_PASS_OWNED);
    ultra_worm->speed(2);
    ultra_worm->behavior_id(1);

    // Change blue worm stats
    EntityEnemy* ultra_worm_stats = reinterpret_cast<EntityEnemy*>(world.entity_type(ENEMY_WORM_3));
    ultra_worm_stats->dropped_item(world.item(ITEM_SAFETY_PASS));
    ultra_worm_stats->drop_probability(1);
    ultra_worm_stats->health(150);

    // Make the platform replacing the boulder slightly higher to prevent caging the snake in the boulder hole
    map->entity(6)->position().half_z = true;
}

void edit_safety_pass_arc(World& world, md::ROM& rom)
{
    setup_ryuma_maps(world);
    edit_ryuma_spawn_house(world);
    edit_ryuma_exterior(world);
    edit_ryuma_pier(world);
    edit_route_to_ryuma_2(world);
    edit_kado_climb_map(world);
    edit_kado_textlines(world);
    edit_helga_swamp_map(world);
    edit_helga_swamp_map_2(world);
    fix_helga_hut_palette(world);
    edit_helga_dialogue(world, rom);
    add_ultra_worm_to_gumi_boulder_map(world);
}
