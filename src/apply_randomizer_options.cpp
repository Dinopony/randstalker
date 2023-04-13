#include "randomizer_options.hpp"

#include <landstalker_lib/model/world.hpp>
#include <landstalker_lib/model/item.hpp>
#include <landstalker_lib/model/entity_type.hpp>
#include "logic_model/world_teleport_tree.hpp"
#include <landstalker_lib/constants/item_codes.hpp>
#include <landstalker_lib/constants/flags.hpp>
#include <landstalker_lib/exceptions.hpp>

#include "assets/game_strings.json.hxx"

#include "logic_model/world_path.hpp"
#include "logic_model/randomizer_world.hpp"
#include "logic_model/item_distribution.hpp"
#include "logic_model/hint_source.hpp"

static void patch_starting_flags(World& world, const RandomizerOptions& options)
{
    std::vector<Flag> flags_to_add = { 
        FLAG_NIGEL_AWOKEN_FROM_KNC_WATERFALL,
        FLAG_INTRO_FINISHED,
        FLAG_KEYDOOR_OPENED_IN_KNC,
        FLAG_FARA_KIDNAPPED,
        FLAG_TALKED_WITH_HIDDEN_VILLAGER_RYUMA,
        FLAG_SPIED_ON_GUMI_RITUAL,
        FLAG_BRIDGE_TO_GUMI_REPAIRED,
        FLAG_MASSAN_VILLAGER_WARNED_BRIDGE_IS_BROKEN,
        FLAG_FIRST_ENCOUNTER_WITH_DUKE,
        FLAG_SAW_FRIDAY_CUTSCENE_AT_MADAME_YARD,
        FLAG_ARTHUR_GAVE_ACCESS_TO_CASTLE,
        FLAG_TALKED_WITH_MERCATOR_GUARDS_ASKING_GOLDS,
        FLAG_SAW_KAYLA_RAFT_CUTSCENE_AT_RYUMA,
        FLAG_ALLOWED_TO_GO_IN_THIEVES_HIDEOUT,
        FLAG_TALKED_WITH_MADAME_YARD_GIRL_HINTING_FORTUNE_TELLER,
        FLAG_SAW_ARTHUR_AT_CASTLE_BACKDOOR,
        FLAG_ASSISTED_TO_MERCATOR_BANQUET,
        FLAG_TALKED_WITH_KITCHEN_WOMAN_BEFORE_BANQUET,
        FLAG_SERVANT_BORUGHT_US_TO_BEDROOM,
        FLAG_DUKE_LEFT_ROOM_AFTER_FIRST_ENCOUNTER,
        FLAG_COMPLETED_MERCATOR_JARS_SIDEQUEST_TWO_TIMES,
        FLAG_SAW_TRAPPED_KAYLA_CUTSCENE_IN_DUNGEON,
        FLAG_THREW_IN_DUNGEON_BY_DUKE,
        FLAG_SAW_KAYLA_LITHOGRAPH_CUTSCENE,
        FLAG_SLEPT_AT_MERCATOR_AFTER_BANQUET,
        FLAG_TOOK_PURPLE_JEWEL_IN_MIR_TOWER,
        FLAG_SAW_MIRO_CLOSE_KEYDOOR,
        FLAG_TALKED_WITH_ARTHUR_REVEALING_DUKE_ESCAPE,
        FLAG_ZAK_FLEW_OFF_MERCATOR_DUNGEON,
        FLAG_SAW_PRINCESS_LARA_IN_MERCATOR_DUNGEON,
        FLAG_PRISONERS_OPENED_UNFINISHED_TUNNEL,
        FLAG_SAW_DUKE_MERCATOR_ON_RAFT_NEAR_DESTEL,
        FLAG_SAW_DEXTER_DIE_IN_CRYPT,
        FLAG_SAW_KAYLA_CUTSCENE_IN_GREENMAZE,
        FLAG_FRIDAY_TALKED_AFTER_GETTING_LANTERN,
        FLAG_SAW_ZAK_ON_BRIDGE,
        FLAG_SAW_DUKE_ENTERING_LAKE_SHRINE,
        FLAG_SAW_DUKE_TAUNTING_IN_SHELL_BREAST_ROOM,
        FLAG_SECOND_RAFT_PLACED_IN_KNL,
        FLAG_SAW_KID_LEAVING_MONSTER_ZOO,
        FLAG_PROSPERO_NOISY_BOY_TEXTLINE_READ,

        // Various keydoors...
        Flag(0x06, 6), Flag(0x06, 5), Flag(0x06, 4), Flag(0x06, 3), Flag(0x06, 2),
        Flag(0x07, 7), Flag(0x07, 5), Flag(0x07, 4), Flag(0x07, 2),
        Flag(0x08, 7), Flag(0x08, 6), Flag(0x08, 5), Flag(0x08, 4), Flag(0x08, 3), Flag(0x08, 2), Flag(0x08, 1),
        Flag(0x09, 6), Flag(0x09, 5), Flag(0x09, 4), Flag(0x09, 3), Flag(0x09, 2),
    };
     
    // If trees are considered visited at start, set all flags related to entering teleport trees
    if(options.all_trees_visited_at_start())
    {
        for(uint8_t bit=0 ; bit<8 ; ++bit)
            flags_to_add.emplace_back(Flag(0x23, bit));
        flags_to_add.emplace_back(Flag(0x25, 0));
        flags_to_add.emplace_back(Flag(0x25, 1));
    }

    // Mark the boulder as already removed at game start
    if(options.remove_gumi_boulder())
        flags_to_add.emplace_back(Flag(0x02, 6));

    world.starting_flags().insert(world.starting_flags().end(), flags_to_add.begin(), flags_to_add.end());
}

static void patch_items(World& world, const RandomizerOptions& options)
{
    // Patch model if user specified a model patch
    const Json& model_patch = options.items_model_patch();
    for(auto& [id_string, patch_json] : model_patch.items())
    {
        uint8_t id = std::stoi(id_string);
        if(world.items().count(id))
            world.item(id)->apply_json(patch_json);
        else
            world.add_item(Item::from_json(id, patch_json));
    }

    if(options.jewel_count() > MAX_INDIVIDUAL_JEWELS)
    {
        Item* red_jewel = world.item(ITEM_RED_JEWEL);
        red_jewel->name("Kazalt Jewel");
        red_jewel->max_quantity(options.jewel_count());
    }
    else
    {
        if(options.jewel_count() >= 5)
        {
            world.item(ITEM_YELLOW_JEWEL)->name("Yellow Jewel");
            world.item(ITEM_YELLOW_JEWEL)->gold_value(500);
            world.item(ITEM_BLUE_JEWEL)->max_quantity(1);
        }
        if(options.jewel_count() >= 4)
        {
            world.item(ITEM_BLUE_JEWEL)->name("Blue Jewel");
            world.item(ITEM_BLUE_JEWEL)->gold_value(500);
            world.item(ITEM_BLUE_JEWEL)->max_quantity(1);
        }
        if(options.jewel_count() >= 3)
        {
            world.item(ITEM_GREEN_JEWEL)->name("Green Jewel");
            world.item(ITEM_GREEN_JEWEL)->gold_value(500);
            world.item(ITEM_GREEN_JEWEL)->max_quantity(1);
        }
    }

    // Process custom starting quantities for items
    const std::array<uint8_t, ITEM_COUNT>& starting_items = options.starting_items();
    for(uint8_t i=0 ; i<ITEM_COUNT ; ++i)
    {
        try
        {
            world.item(i)->starting_quantity(std::min<uint8_t>(starting_items[i], 9));
        }
        catch(std::out_of_range&) {}
    }

    // Alter a few things depending on settings
    if (options.use_armor_upgrades())
    {
        world.item(ITEM_STEEL_BREAST)->gold_value(250);
        world.item(ITEM_CHROME_BREAST)->gold_value(250);
        world.item(ITEM_SHELL_BREAST)->gold_value(250);
        world.item(ITEM_HYPER_BREAST)->gold_value(250);
    }

    if (options.consumable_record_book())
    {
        world.item(ITEM_RECORD_BOOK)->max_quantity(9);
        uint16_t currentPrice = world.item(ITEM_RECORD_BOOK)->gold_value();
        world.item(ITEM_RECORD_BOOK)->gold_value(currentPrice / 5);
    }

    if (options.consumable_spell_book())
    {
        world.item(ITEM_SPELL_BOOK)->max_quantity(9);
        uint16_t currentPrice = world.item(ITEM_SPELL_BOOK)->gold_value();
        world.item(ITEM_SPELL_BOOK)->gold_value(currentPrice / 5);
    }
}

static void patch_entity_types(World& world, const RandomizerOptions& options)
{
    // Apply multiplicators from randomizer settings
    for (auto& [id, entity_type] : world.entity_types())
    {
        if(entity_type->type_name() != "enemy")
            continue;
        EnemyType* enemy_type = reinterpret_cast<EnemyType*>(entity_type);

        enemy_type->apply_damage_factor(options.enemies_damage_factor());
        enemy_type->apply_health_factor(options.enemies_health_factor());
        enemy_type->apply_armor_factor(options.enemies_armor_factor());
        enemy_type->apply_golds_factor(options.enemies_golds_factor());
        enemy_type->apply_drop_chance_factor(options.enemies_drop_chance_factor());
    }
}

static void patch_game_strings(World& world, const RandomizerOptions& options)
{
    Json game_strings_json = Json::parse(GAME_STRINGS_JSON, nullptr, true, true);
    std::vector<std::string> strings_to_empty = game_strings_json.at("emptiedIndices");
    for(const std::string& string_hex_id : strings_to_empty)
    {
        uint16_t game_string_id = std::stoi(string_hex_id, nullptr, 16);
        world.game_strings()[game_string_id] = "";
    }

    for (auto& [string_hex_id, string_value] : game_strings_json.at("patches").items())
    {
        uint16_t game_string_id = std::stoi(string_hex_id, nullptr, 16);
        world.game_strings()[game_string_id] = string_value;
    }

    // Kazalt rejection message
    world.game_strings()[0x022] = std::string("Only the bearers of the ") 
        + std::to_string(options.jewel_count()) + " jewels\n are worthy of entering\n King Nole's domain...\x1E";
}

static void apply_options_on_logic_paths(const RandomizerOptions& options, RandomizerWorld& world)
{
    // Handle paths related to specific tricks
    if(options.remove_gumi_boulder())
        world.add_path(new WorldPath(world.node("route_gumi_ryuma"), world.node("gumi")));

    if(options.handle_enemy_jumping_in_logic())
    {
        // Mountainous Area can be reached from route to Lake Shrine by doing a "ghost jump" at crossroads map
        world.add_path(new WorldPath(world.node("route_lake_shrine"), world.node("route_lake_shrine_cliff")));
        // Mir Tower sector first tree can be bypassed by jumping on an enemy to reach the elevated ledge
        world.path(world.node("mir_tower_sector"), world.node("mir_tower_sector_tree_ledge"))->required_items().clear();
    }

    if(options.handle_tree_cutting_glitch_in_logic())
    {
        // Both trees in Mir Tower sector can be abused through tree cutting glitch
        world.path(world.node("mir_tower_sector"), world.node("mir_tower_sector_tree_coast"))->required_items().clear();
        world.path(world.node("mir_tower_sector"), world.node("mir_tower_sector_tree_ledge"))->required_items().clear();
    }

    // If damage boosting is taken in account in logic, remove all iron boots & fireproof requirements
    if(options.handle_damage_boosting_in_logic())
    {
        for(auto& [pair, path] : world.paths())
        {
            std::vector<Item*>& required_items = path->required_items();

            auto it = std::find(required_items.begin(), required_items.end(), world.item(ITEM_IRON_BOOTS));
            if(it != required_items.end())
                required_items.erase(it);

            it = std::find(required_items.begin(), required_items.end(), world.item(ITEM_FIREPROOF_BOOTS));
            if(it != required_items.end())
                required_items.erase(it);
        }
    }

    // If using Einstein Whistle behind trees is allowed, add a new logic path there to reflect that change
    if(options.allow_whistle_usage_behind_trees())
    {
        world.add_path(new WorldPath(
            world.node("greenmaze_post_whistle"),
            world.node("greenmaze_pre_whistle"),
            1,
            { world.item(ITEM_EINSTEIN_WHISTLE) }
        ));
    }

    // Determine the list of required jewels to go from King Nole's Cave to Kazalt depending on settings
    WorldPath* path_to_kazalt = world.path("king_nole_cave", "kazalt");
    if(options.jewel_count() > MAX_INDIVIDUAL_JEWELS)
    {
        for(int i=0; i<options.jewel_count() ; ++i)
            path_to_kazalt->add_required_item(world.item(ITEM_RED_JEWEL));
    }
    else if(options.jewel_count() >= 1)
    {
        path_to_kazalt->add_required_item(world.item(ITEM_RED_JEWEL));
        if(options.jewel_count() >= 2)
            path_to_kazalt->add_required_item(world.item(ITEM_PURPLE_JEWEL));
        if(options.jewel_count() >= 3)
            path_to_kazalt->add_required_item(world.item(ITEM_GREEN_JEWEL));
        if(options.jewel_count() >= 4)
            path_to_kazalt->add_required_item(world.item(ITEM_BLUE_JEWEL));
        if(options.jewel_count() >= 5)
            path_to_kazalt->add_required_item(world.item(ITEM_YELLOW_JEWEL));
    }
}

static void apply_options_on_spawn_locations(const RandomizerOptions& options, RandomizerWorld& world)
{
    // Patch model if user specified a model patch
    const Json& model_patch = options.spawn_locations_model_patch();
    for(auto& [id, patch_json] : model_patch.items())
    {
        if(!world.available_spawn_locations().count(id))
            world.add_spawn_location(SpawnLocation::from_json(id, patch_json));
        else
            world.available_spawn_locations().at(id)->apply_json(patch_json);
    }
}

static void apply_options_on_hint_sources(const RandomizerOptions& options, RandomizerWorld& world)
{
    // Patch model if user specified a model patch
    const Json& model_patch = options.hint_sources_model_patch();
    for(const Json& json : model_patch)
    {
        world.add_hint_source(HintSource::from_json(json, world.nodes()));
    }
}

static void apply_options_on_item_distributions(const RandomizerOptions& options, RandomizerWorld& world)
{
    // Apply the global distribution params, if set by the user
    const std::array<uint8_t, ITEM_COUNT+1>& distribution_param = options.items_distribution();
    for(uint8_t i=0 ; i<ITEM_COUNT+1 ; ++i)
        world.item_distribution(i)->quantity(distribution_param[i]);

    // Apply other params that indirectly influence item distribution
    if(options.jewel_count() > MAX_INDIVIDUAL_JEWELS)
    {
        world.item_distribution(ITEM_RED_JEWEL)->allowed_on_ground(false);
        if(world.item_distribution(ITEM_RED_JEWEL)->quantity() == 0)
        {
            world.item_distribution(ITEM_RED_JEWEL)->add(options.jewel_count());
            world.item_distribution(ITEM_NONE)->remove(options.jewel_count());
        }
    }
    else
    {
        if(options.jewel_count() >= 1 && world.item_distribution(ITEM_RED_JEWEL)->quantity() == 0)
        {
            world.item_distribution(ITEM_RED_JEWEL)->add(1);
            world.item_distribution(ITEM_NONE)->remove(1);
        }
        if(options.jewel_count() >= 2 && world.item_distribution(ITEM_PURPLE_JEWEL)->quantity() == 0)
        {
            world.item_distribution(ITEM_PURPLE_JEWEL)->add(1);
            world.item_distribution(ITEM_NONE)->remove(1);
        }
        if(options.jewel_count() >= 3 && world.item_distribution(ITEM_GREEN_JEWEL)->quantity() == 0)
        {
            world.item_distribution(ITEM_GREEN_JEWEL)->add(1);
            world.item_distribution(ITEM_NONE)->remove(1);
        }
        if(options.jewel_count() >= 4 && world.item_distribution(ITEM_BLUE_JEWEL)->quantity() == 0)
        {
            world.item_distribution(ITEM_BLUE_JEWEL)->add(1);
            world.item_distribution(ITEM_NONE)->remove(1);
        }
        if(options.jewel_count() >= 5 && world.item_distribution(ITEM_YELLOW_JEWEL)->quantity() == 0)
        {
            world.item_distribution(ITEM_YELLOW_JEWEL)->add(1);
            world.item_distribution(ITEM_NONE)->remove(1);
        }
    }
}

void apply_randomizer_options(const RandomizerOptions& options, RandomizerWorld& world)
{
    world.starting_golds(options.starting_gold());
    world.starting_life(options.starting_life());

    patch_starting_flags(world, options);
    patch_items(world, options);
    patch_entity_types(world, options);
    patch_game_strings(world, options);

    apply_options_on_logic_paths(options, world);
    apply_options_on_spawn_locations(options, world);
    apply_options_on_item_distributions(options, world);
    apply_options_on_hint_sources(options, world);
}
