#pragma once

#include <landstalker_lib/patches/game_patch.hpp>

#include <landstalker_lib/model/world.hpp>
#include <landstalker_lib/model/map.hpp>
#include <landstalker_lib/model/entity.hpp>
#include <landstalker_lib/model/entity_type.hpp>
#include <landstalker_lib/constants/map_codes.hpp>
#include <landstalker_lib/constants/entity_type_codes.hpp>
#include <landstalker_lib/constants/flags.hpp>

class PatchRandoWorldEdits : public GamePatch
{
public:
    void alter_world(World& world) override
    {
        put_orcs_back_in_room_before_boss_swamp_shrine(world);
        make_arthur_always_in_throne_room(world);
        make_mercator_docks_shop_always_open(world);
        replace_sick_merchant_by_chest(world);
        alter_mercator_special_shop_check(world);
        add_reverse_mercator_gate(world);
        remove_sailor_in_dark_port(world);
        remove_mercator_castle_backdoor_guard(world);
        alter_palettes_for_miscolored_ground_items(world);
        remove_logs_room_exit_check(world);
        make_pockets_always_in_thieves_hideout_cell(world);
        remove_pockets_from_gumi(world);
        put_back_giants_in_verla_mines_keydoor_map(world);
        put_back_golems_in_lake_shrine_keydoor_map(world);
        remove_verla_soldiers_on_verla_spawn(world);
        make_verla_mines_bosses_always_present(world);
    }

private:
    static void put_orcs_back_in_room_before_boss_swamp_shrine(World& world)
    {
        // In map before boss, put the orcs back and only remove the locked door
        world.map(MAP_SWAMP_SHRINE_ROOM_BEFORE_BOSS)->global_entity_mask_flags().clear();
        world.map(MAP_SWAMP_SHRINE_ROOM_BEFORE_BOSS)->remove_entity(5);
        // Remove the door orcs if Fara has been freed
        world.map(MAP_SWAMP_SHRINE_ROOM_BEFORE_BOSS)->entity(1)->mask_flags().emplace_back(EntityMaskFlag(false, 2, 5));
        world.map(MAP_SWAMP_SHRINE_ROOM_BEFORE_BOSS)->entity(2)->mask_flags().emplace_back(EntityMaskFlag(false, 2, 5));
    }

    static void make_arthur_always_in_throne_room(World& world)
    {
        // Make Arthur and the boy next to him always present in their throne room variant
        world.map(MAP_MERCATOR_CASTLE_THRONE_ROOM)->entity(0)->mask_flags().clear();
        world.map(MAP_MERCATOR_CASTLE_THRONE_ROOM)->entity(1)->mask_flags().clear();

        // Remove Arthur from the armory
        world.map(MAP_MERCATOR_CASTLE_ARMORY_1F)->variants().clear();
        world.map(MAP_MERCATOR_CASTLE_ARMORY_1F)->clear_entities();
        world.map(MAP_MERCATOR_CASTLE_ARMORY_1F_VARIANT)->clear();
    }

    static void make_mercator_docks_shop_always_open(World& world)
    {
        // Mercator docks (lighthouse fixed variant) has a clear flag that makes the shop
        // disappear once you reach a certain point in the scenario. We remove this clear flag
        // to prevent any potential softlock.
        world.map(MAP_MERCATOR_DOCKS_LIGHTHOUSE_FIXED_VARIANT)->global_entity_mask_flags().clear();
    }

    static void replace_sick_merchant_by_chest(World& world)
    {
        // Set the index for added chest in map to "0E" instead of "C2"
        world.map(MAP_MERCATOR_SPECIAL_SHOP_BACKROOM)->base_chest_id(0x0E);

        // Remove the sick merchant
        world.map(MAP_MERCATOR_SPECIAL_SHOP_BACKROOM)->remove_entity(0);

        // Add a chest
        world.map(MAP_MERCATOR_SPECIAL_SHOP_BACKROOM)->add_entity(new Entity({
                                                                                     .type_id = ENTITY_CHEST,
                                                                                     .position = Position(14, 18, 0),
                                                                                     .orientation = ENTITY_ORIENTATION_NW,
                                                                                     .palette = 2
                                                                             }));

        // Move the kid to hide the fact that the bed looks broken af
        world.map(MAP_MERCATOR_SPECIAL_SHOP_BACKROOM)->entity(0)->position().y = 21;

//   Bed tile GFX swap?
//   rom.set_word(0x0050B4, 0x0008); // Before: 0x2A0C (bit 4 of 102A) | After: 0x0008 (bit 0 of 1000 - always true)
    }

    /**
     * Usually, when entering Mercator special shop, the game checks if we completed the
     * dock traders sidequest. We chose to make the appearance of this shop bound to owning
     * Buyer's Card, since it's an item we usually get during that sidequest.
     */
    static void alter_mercator_special_shop_check(World& world)
    {
        // Remove the second shop variant that is not used in the rando (and clear the map to make room for other data)
        Map* variant_to_erase = world.map(MAP_MERCATOR_SPECIAL_SHOP_VARIETY_VARIANT);
        world.map(MAP_MERCATOR_SPECIAL_SHOP)->variants().erase(variant_to_erase);
        variant_to_erase->clear();

        // Make the remaining special shop variant triggered by owning Buyer's Card
        Map* variant_to_alter = world.map(MAP_MERCATOR_SPECIAL_SHOP_CONSUMABLE_VARIANT);
        Flag& variant_flag = world.map(MAP_MERCATOR_SPECIAL_SHOP)->variants().at(variant_to_alter);
        variant_flag.byte = 0x4C;
        variant_flag.bit = 5;
    }

    /**
     * Add a door inside Mercator enforcing that Safety Pass is owned to allow exiting from the front gate
     */
    static void add_reverse_mercator_gate(World& world)
    {
        // Alter map variants so that we are stuck in the base map as long as Safety Pass is not owned
        Flag& variant_1_flag = world.map(MAP_MERCATOR_EXTERIOR_1)->variant(world.map(MAP_MERCATOR_EXTERIOR_1_VARIANT_1));
        variant_1_flag.byte = 0x59;
        variant_1_flag.bit = 5;

        Flag& variant_2_flag = world.map(MAP_MERCATOR_EXTERIOR_1)->variant(world.map(MAP_MERCATOR_EXTERIOR_1_VARIANT_2));
        variant_2_flag.byte = 0x59;
        variant_2_flag.bit = 5;

        // Add doors in the non-safety pass variant blocking the way out of Mercator
        Entity* door = world.map(MAP_MERCATOR_EXTERIOR_1)->add_entity(new Entity({
                                                                                         .type_id = ENTITY_GATE_SOUTH,
                                                                                         .position = Position(48, 41, 1, false, true, false),
                                                                                         .orientation = ENTITY_ORIENTATION_SE
                                                                                 }));

        world.map(MAP_MERCATOR_EXTERIOR_1)->add_entity(new Entity({
                                                                          .type_id = ENTITY_GATE_SOUTH,
                                                                          .position = Position(48, 43, 1, false, true, false),
                                                                          .orientation = ENTITY_ORIENTATION_SE,
                                                                          .entity_to_use_tiles_from = door
                                                                  }));
    }

    /**
     * There is a sailor NPC in the "dark" version of Mercator port who responds badly to story triggers,
     * allowing us to sail to Verla even without having repaired the lighthouse. To prevent this from being exploited,
     * we removed him altogether.
     */
    static void remove_sailor_in_dark_port(World& world)
    {
        world.map(MAP_MERCATOR_DOCKS_DARK_VARIANT)->remove_entity(1);
    }

    /**
     * There is a guard staying in front of the Mercator castle backdoor to prevent you from using
     * Mir Tower keys on it. He appears when Crypt is finished and disappears when Mir Tower is finished,
     * but we actually never want him to be there, so we delete it.
     */
    static void remove_mercator_castle_backdoor_guard(World& world)
    {
        world.map(MAP_MERCATOR_FOUNTAIN)->remove_entity(5);
    }

    static void alter_palettes_for_miscolored_ground_items(World& world)
    {
        // Force trees to use environmental palette
        for(uint8_t i=2 ; i<=5 ; ++i)
            world.map(MAP_GREENMAZE_SUNSTONE)->entities().at(i)->palette(0);
        // Put item back on its appropriate palette
        world.map(MAP_GREENMAZE_SUNSTONE)->entities().at(0)->palette(1);

        // Force tree to use environmental palette
        world.map(MAP_KN_LABYRINTH_TREE_LOGS)->entities().at(0)->palette(0);
        // Put items back on their appropriate palette
        for(uint8_t i=1 ; i<=2 ; ++i)
            world.map(MAP_KN_LABYRINTH_TREE_LOGS)->entities().at(i)->palette(1);
    }

    /**
     * Usually, when trying to leave the room where you get Logs in the vanilla game without having taken both logs, a dwarf
     * comes and prevents you from leaving. Here, we remove that check since we cannot softlock anymore on the raft.
     */
    static void remove_logs_room_exit_check(World& world)
    {
        world.map(MAP_KN_LABYRINTH_TREE_LOGS)->remove_entity(3);
    }

    static void make_pockets_always_in_thieves_hideout_cell(World& world)
    {
        world.map(MAP_THIEVES_HIDEOUT_POCKETS_CELL)->entity(2)->mask_flags().clear();
    }

    static void remove_pockets_from_gumi(World& world)
    {
        world.map(MAP_GUMI_EXTERIOR_VARIANT_DESERTED)->remove_entity(3);
    }

    /**
     * In the original game, having the keydoor open near Verla Mines entrance remove permanently the brown giants from
     * the room. In rando, this door is always open through the flag and therefore giants were never there.
     * This patch makes them come back, even when the door is open.
     */
    static void put_back_giants_in_verla_mines_keydoor_map(World& world)
    {
        Map* map = world.map(MAP_VERLA_MINES_KEYDOOR_NEAR_ENTRANCE);
        map->global_entity_mask_flags().clear();
        map->key_door_mask_flags().clear();
        // Remove the keydoor and an invisible cube
        map->remove_entity(1);
        map->remove_entity(0);
    }

    /**
     * In the original game, having the keydoor open near Lake Shrine entrance remove permanently the green golems
     * from the room. In rando, this door is always open through the flag and therefore golems were never there, leaving
     * a useless switch in the room. This patch makes them come back, even when the door is open.
     */
    static void put_back_golems_in_lake_shrine_keydoor_map(World& world)
    {
        Map* map = world.map(MAP_LAKE_SHRINE_0F_KEYDOOR);
        map->global_entity_mask_flags().clear();
        map->key_door_mask_flags().clear();
        map->entity(4)->mask_flags().emplace_back(EntityMaskFlag(false, 7, 5));
    }

    /**
     * This sub-patch removes the knights attacking the player in Verla if it is the spawn location for the seed.
     */
    static void remove_verla_soldiers_on_verla_spawn(World& world)
    {
        RandomizerWorld& randomizer_world = reinterpret_cast<RandomizerWorld&>(world);
        if(randomizer_world.spawn_location()->node_id() == "verla")
        {
            world.starting_flags().emplace_back(FLAG_VERLA_WHITE_KNIGHTS_KILLED);
            world.starting_flags().emplace_back(FLAG_VERLA_GOLDEN_KNIGHT_KILLED);
        }
    }

    static void make_verla_mines_bosses_always_present(World& world)
    {
        // --- Make Slasher appearance not dependant on Marley being killed ---
        world.map(MAP_VERLA_MINES_SLASHER_ARENA)->global_entity_mask_flags().clear();

        // --- Make Dex present, even though story flags force him to disappear ---
        Map* map = world.map(MAP_VERLA_MINES_DEX_ARENA);

        // Clear global masks
        map->global_entity_mask_flags().clear();

        // Remove townsfolk and invisible cube
        map->remove_entity(4);
        map->remove_entity(3);
        map->remove_entity(2);
        map->remove_entity(1);

        // Add a new door
        map->add_entity(new Entity({
            .type_id = ENTITY_GATE_NORTH,
            .position = Position(20, 17, 3),
            .behavior_id = 525, // Open when Dex is killed
            .mask_flags = { EntityMaskFlag(false, FLAG_DEX_KILLED) },
        }));
    }
};
