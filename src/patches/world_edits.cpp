#include "../world.hpp"

#include "../tools/megadrive/rom.hpp"
#include "../tools/megadrive/code.hpp"

#include "../randomizer_options.hpp"
#include "../model/map.hpp"
#include "../model/entity.hpp"
#include "../model/entity_type.hpp"

void handle_additionnal_jewels(World& world)
{
    // Remove jewels replaced from book IDs from priest stands
    world.map(MAP_MASSAN_CHURCH)->remove_entity(3);
    world.map(MAP_MASSAN_CHURCH)->remove_entity(2);
    world.map(MAP_GUMI_CHURCH)->remove_entity(2);
    world.map(MAP_GUMI_CHURCH)->remove_entity(1);
    world.map(MAP_RYUMA_CHURCH)->remove_entity(4);
    world.map(MAP_RYUMA_CHURCH)->remove_entity(3);
    world.map(MAP_MERCATOR_CHURCH_VARIANT)->remove_entity(3);
    world.map(MAP_MERCATOR_CHURCH_VARIANT)->remove_entity(2);
    world.map(MAP_VERLA_CHURCH)->remove_entity(3);
    world.map(MAP_VERLA_CHURCH)->remove_entity(2);
    world.map(MAP_DESTEL_CHURCH)->remove_entity(3);
    world.map(MAP_DESTEL_CHURCH)->remove_entity(2);
    world.map(MAP_KAZALT_CHURCH)->remove_entity(3);
    world.map(MAP_KAZALT_CHURCH)->remove_entity(2);
}

void put_orcs_back_in_room_before_boss_swamp_shrine(World& world)
{
    // In map before boss, put the orcs back and only remove the locked door
    world.map(MAP_SWAMP_SHRINE_ROOM_BEFORE_BOSS)->global_entity_mask_flags().clear();
    world.map(MAP_SWAMP_SHRINE_ROOM_BEFORE_BOSS)->remove_entity(5);
    // Remove the door orcs if Fara has been freed
    world.map(MAP_SWAMP_SHRINE_ROOM_BEFORE_BOSS)->entity(1)->mask_flags().push_back(EntityMaskFlag(false, 2, 5));
    world.map(MAP_SWAMP_SHRINE_ROOM_BEFORE_BOSS)->entity(2)->mask_flags().push_back(EntityMaskFlag(false, 2, 5));
}

/**
 * Make it so Lifestock chest near Fara in Swamp Shrine appears again when going back into the room afterwards, preventing any softlock there.
 */
void fix_fara_arena_chest_softlock(World& world, md::ROM& rom)
{
    // Move the chest to the ground
    Entity* chest = world.map(MAP_SWAMP_SHRINE_BOSS_ROOM)->entity(14);
    chest->position(26, 19, 4);
    chest->behavior_id(0);
    
    // --------- Function to remove all entities but the chest when coming back in the room ---------
    md::Code func_remove_all_entities_but_chest_in_fara_room;

    func_remove_all_entities_but_chest_in_fara_room.movem_to_stack({ reg_D0_D7 }, { reg_A0_A6 });
    func_remove_all_entities_but_chest_in_fara_room.lea(0xFF5480, reg_A0);
    func_remove_all_entities_but_chest_in_fara_room.moveq(0xD, reg_D0);
    func_remove_all_entities_but_chest_in_fara_room.label("loop_remove_entities");
        func_remove_all_entities_but_chest_in_fara_room.movew(0x7F7F, addr_(reg_A0));
        func_remove_all_entities_but_chest_in_fara_room.adda(0x80, reg_A0);
    func_remove_all_entities_but_chest_in_fara_room.dbra(reg_D0, "loop_remove_entities");
    func_remove_all_entities_but_chest_in_fara_room.movem_from_stack({ reg_D0_D7 }, { reg_A0_A6 });
    func_remove_all_entities_but_chest_in_fara_room.rts();

    uint32_t addr = rom.inject_code(func_remove_all_entities_but_chest_in_fara_room);

    // Call the injected function
    rom.set_code(0x019BE0, md::Code().jsr(addr).nop());
}

void make_gumi_boulder_push_not_story_dependant(World& world)
{
    // Turn the last boulder into a platform to prevent any softlock
    Entity* last_boulder = world.map(MAP_ROUTE_GUMI_RYUMA_BOULDER)->entity(6);
    last_boulder->entity_type_id(world.entity_type("large_wood_platform")->id());
    last_boulder->entity_to_use_tiles_from(nullptr);
    last_boulder->half_tile_z(true);
    last_boulder->palette(2);

    // Always remove Pockets from Gumi boulder map
    world.map(MAP_ROUTE_GUMI_RYUMA_BOULDER)->remove_entity(4);

    // Erase the entity masks which make bears only appear under a specific scenario requirement
    world.map(MAP_ROUTE_GUMI_RYUMA_BOULDER)->entity(2)->mask_flags().clear();
    world.map(MAP_ROUTE_GUMI_RYUMA_BOULDER)->entity(3)->mask_flags().clear();
}

/**
 * In the original game, coming back to Mir room after Lake Shrine would softlock you because Mir
 * would not be there. This check is removed to prevent any softlock and allow fighting Mir after having
 * done Lake Shrine.
 */
void fix_mir_after_lake_shrine_softlock(World& world)
{
    world.map(MAP_MIR_TOWER_BOSS_ARENA)->global_entity_mask_flags().clear();
}

void make_arthur_always_in_throne_room(World& world)
{
    // Make Arthur and the boy next to him always present in their throne room variant
    world.map(MAP_MERCATOR_CASTLE_THRONE_ROOM_ARTHUR_VARIANT)->entity(0)->mask_flags().clear();
    world.map(MAP_MERCATOR_CASTLE_THRONE_ROOM_ARTHUR_VARIANT)->entity(1)->mask_flags().clear();

    // Remove Arthur from the armory
    world.map(MAP_MERCATOR_CASTLE_ARMORY_1F)->remove_entity(0);
}

void make_mercator_docks_shop_always_open(World& world)
{
    // Mercator docks (lighthouse fixed variant) has a clear flag that makes the shop
    // disappear once you reach a certain point in the scenario. We remove this clear flag
    // to prevent any potential softlock.
    world.map(MAP_MERCATOR_DOCKS_LIGHTHOUSE_FIXED_VARIANT)->global_entity_mask_flags().clear();
}

void make_ryuma_shop_always_open(World& world)
{
    // Make Ryuma's shop open before saving the mayor from Thieves Hideout
    world.map(MAP_RYUMA_SHOP)->entity(0)->mask_flags().clear();
}

/**
 * The "falling ribbon" item source in Mercator castle court is pretty dependant on story flags. In the original game,
 * the timeframe in the story where we can get it is tight. We get rid of any condition here, apart from checking
 * if item has already been obtained.
 */
void make_falling_ribbon_not_story_dependant(World& world)
{
    // Remove use of map variant where the ribbon does not exist, and empty the now unused variant
    world.map(MAP_MERCATOR_CASTLE_LEFT_COURT)->variants().clear();
    world.map(MAP_MERCATOR_CASTLE_LEFT_COURT_RIBBONLESS_VARIANT)->clear();

    // Remove an entity mask preventing us from getting the ribbon once we get too far in the story
    std::vector<EntityMaskFlag>& ribbon_masks = world.map(MAP_MERCATOR_CASTLE_LEFT_COURT)->entity(2)->mask_flags();
    ribbon_masks.erase(ribbon_masks.begin() + ribbon_masks.size()-1);

    // Remove the servant guarding the door
    world.map(MAP_MERCATOR_CASTLE_LEFT_COURT)->remove_entity(0);
}

void replace_sick_merchant_by_chest(World& world)
{
    // Set the index for added chest in map to "0E" instead of "C2"
    world.map(MAP_MERCATOR_SPECIAL_SHOP_BACKROOM)->base_chest_id(0x0E);

    // Remove the sick merchant
    world.map(MAP_MERCATOR_SPECIAL_SHOP_BACKROOM)->remove_entity(0);

    // Add a chest
    Entity* new_chest = new Entity();
    new_chest->entity_type_id(0x12);
    new_chest->position(14, 18, 0);
    new_chest->orientation(ENTITY_ORIENTATION_NW);
    new_chest->palette(2);
    world.map(MAP_MERCATOR_SPECIAL_SHOP_BACKROOM)->add_entity(new_chest);

    // Move the kid to hide the fact that the bed looks broken af
    world.map(MAP_MERCATOR_SPECIAL_SHOP_BACKROOM)->entity(0)->pos_y(21);

//   Bed tile GFX swap?
//   rom.set_word(0x0050B4, 0x0008); // Before: 0x2A0C (bit 4 of 102A) | After: 0x0008 (bit 0 of 1000 - always true)
}

/**
 * Usually, when entering Mercator special shop, the game checks if we completed the 
 * dock traders sidequest. We chose to make the appearance of this shop bound to owning
 * Buyer's Card, since it's an item we usually get during that sidequest.
 */
void alter_mercator_special_shop_check(World& world)
{
    std::vector<MapVariant>& special_shop_variants = world.map(MAP_MERCATOR_SPECIAL_SHOP)->variants();
    
    // Remove the second shop variant that is not used in the rando (and clear the map to make room for other data)
    special_shop_variants.erase(special_shop_variants.begin());
    world.map(MAP_MERCATOR_SPECIAL_SHOP_VARIETY_VARIANT)->clear();

    // Make the remaining special shop variant triggered by owning Buyer's Card
    special_shop_variants[0].flag_byte = 0x4C;
    special_shop_variants[0].flag_bit = 5;
}

/**
 * Add a door inside Mercator enforcing that Safety Pass is owned to allow exiting from the front gate
 */
void add_reverse_mercator_gate(World& world)
{
    // Alter map variants so that we are stuck in the base map as long as Safety Pass is not owned
    MapVariant& variant1 = world.map(MAP_MERCATOR_ENTRANCE)->variant(MAP_MERCATOR_ENTRANCE_VARIANT_1);
    variant1.flag_byte = 0x59;
    variant1.flag_bit = 5;

    MapVariant& variant2 = world.map(MAP_MERCATOR_ENTRANCE)->variant(MAP_MERCATOR_ENTRANCE_VARIANT_2);
    variant2.flag_byte = 0x59;
    variant2.flag_bit = 5;

    // Add doors in the non-safety pass variant blocking the way out of Mercator
    Entity* door1 = new Entity();
    door1->entity_type_id(0x67);
    door1->position(48, 41, 1);
    door1->half_tile_y(true);
    door1->orientation(ENTITY_ORIENTATION_SE);
    world.map(MAP_MERCATOR_ENTRANCE)->add_entity(door1);

    Entity* door2 = new Entity(*door1);
    door2->pos_y(door1->pos_y()+2);
    door2->entity_to_use_tiles_from(door1);
    world.map(MAP_MERCATOR_ENTRANCE)->add_entity(door2);
}

/**
 * There is a sailor NPC in the "dark" version of Mercator port who responds badly to story triggers, 
 * allowing us to sail to Verla even without having repaired the lighthouse. To prevent this from being exploited, 
 * we removed him altogether.
 */
void remove_sailor_in_dark_port(World& world)
{
    world.map(MAP_MERCATOR_DOCKS_DARK_VARIANT)->remove_entity(1);
}

/**
 * There is a guard staying in front of the Mercator castle backdoor to prevent you from using
 * Mir Tower keys on it. He appears when Crypt is finished and disappears when Mir Tower is finished,
 * but we actually never want him to be there, so we delete him from existence by moving him away from the map.
 */
void remove_mercator_castle_backdoor_guard(World& world)
{
    world.map(MAP_MERCATOR_FOUNTAIN)->remove_entity(5);
}

/**
 * Make entering the fountain tunnel automatically trigger the fountain button to prevent 
 * any softlock by coming from Greenmaze without having opened the passage.
 */
void fix_reverse_greenmaze_fountain_softlock(World& world)
{
    world.map(MAP_MERCATOR_TUNNEL_TO_GREENMAZE)->visited_flag(Flag(0xE, 1));
}

/**
 * Fix armlet skip by putting the tornado way higher, preventing any kind of buffer-jumping on it
 */
void fix_armlet_skip(World& world)
{
    // Make Mir Tower magic barrier non-colliding
    world.map(MAP_MIR_TOWER_EXTERIOR)->entity(9)->can_pass_through(true);
}

void apply_world_edits(World& world, const RandomizerOptions& options, md::ROM& rom)
{
    if(options.jewel_count() <= MAX_INDIVIDUAL_JEWELS)
        handle_additionnal_jewels(world);

    put_orcs_back_in_room_before_boss_swamp_shrine(world);
    fix_fara_arena_chest_softlock(world, rom);
    make_gumi_boulder_push_not_story_dependant(world);
    fix_mir_after_lake_shrine_softlock(world);
    make_arthur_always_in_throne_room(world);
    make_mercator_docks_shop_always_open(world);
    make_ryuma_shop_always_open(world);
    make_falling_ribbon_not_story_dependant(world);
    replace_sick_merchant_by_chest(world);
    alter_mercator_special_shop_check(world);
    add_reverse_mercator_gate(world);
    remove_sailor_in_dark_port(world);
    remove_mercator_castle_backdoor_guard(world);
    fix_reverse_greenmaze_fountain_softlock(world);

    if(options.fix_armlet_skip())
        fix_armlet_skip(world);
}