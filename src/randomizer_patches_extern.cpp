#include "randomizer_options.hpp"
#include "world_model/world.hpp"
#include "world_model/item.hpp"
#include "world_model/entity_type.hpp"

#include "constants/item_codes.hpp"
#include "constants/flags.hpp"

#include "exceptions.hpp"

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
        FLAG_DEX_KILLED,
        FLAG_PRISONERS_OPENED_UNFINISHED_TUNNEL,
        FLAG_OPENED_MERCATOR_TELEPORTER_TO_KNC,
        FLAG_SAW_DUKE_MERCATOR_ON_RAFT_NEAR_DESTEL,
        FLAG_SAW_DEXTER_DIE_IN_CRYPT,
        FLAG_SAW_KAYLA_CUTSCENE_IN_GREENMAZE,
        FLAG_FRIDAY_TALKED_AFTER_GETTING_LANTERN,
        FLAG_SAW_ZAK_ON_BRIDGE,
        FLAG_SAW_DUKE_ENTERING_LAKE_SHRINE,
        FLAG_SAW_DUKE_TAUNTING_IN_SHELL_BREAST_ROOM,
        FLAG_SECOND_RAFT_PLACED_IN_KNL,

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
            flags_to_add.push_back(Flag(0x23, bit));
        flags_to_add.push_back(Flag(0x25, 0));
        flags_to_add.push_back(Flag(0x25, 1));
    }

    // Clear Verla soldiers if spawning in Verla
    if(world.spawn_location().id() == "verla")
    {
        flags_to_add.push_back(Flag(0x26, 3));
        flags_to_add.push_back(Flag(0x26, 4));
    }

    // Mark the boulder as already removed at game start
    if(options.remove_gumi_boulder())
        flags_to_add.push_back(Flag(0x02, 6));

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

    // Process custom starting quantities for items
    const std::map<std::string, uint8_t>& starting_items = options.starting_items();
    for(auto& [item_name, quantity] : starting_items)
    {
        Item* item = world.item(item_name);
        if(!item)
        {
            std::stringstream msg;
            msg << "Cannot set starting quantity of unknown item '" << item_name << "'";
            throw RandomizerException(msg.str());
        }

        item->starting_quantity(std::min<uint8_t>(quantity, 9));
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

    if(options.jewel_count() > MAX_INDIVIDUAL_JEWELS)
    {
        Item* red_jewel = world.item(ITEM_RED_JEWEL);
        red_jewel->name("Kazalt Jewel");
        red_jewel->allowed_on_ground(false);
        red_jewel->max_quantity(options.jewel_count());
    }
}

static void patch_entity_types(World& world, const RandomizerOptions& options)
{
    // Apply multiplicators from randomizer settings
    for (auto& [id, entity_type] : world.entity_types())
    {
        if(entity_type->type_name() != "enemy")
            continue;
        EntityEnemy* enemy_type = reinterpret_cast<EntityEnemy*>(entity_type);

        enemy_type->apply_damage_factor(options.enemies_damage_factor());
        enemy_type->apply_health_factor(options.enemies_health_factor());
        enemy_type->apply_armor_factor(options.enemies_armor_factor());
        enemy_type->apply_golds_factor(options.enemies_golds_factor());
        enemy_type->apply_drop_chance_factor(options.enemies_drop_chance_factor());
    }
}

static void patch_game_strings(World& world, const RandomizerOptions& options)
{
    // Kazalt rejection message
    world.game_strings()[0x022] = std::string("Only the bearers of the ") 
        + std::to_string(options.jewel_count()) + " jewels\n are worthy of entering\n King Nole's domain...\x1E";
}

void apply_rando_options_to_world(const RandomizerOptions& options, World& world)
{
    world.starting_golds(options.starting_gold());
    world.custom_starting_life(options.starting_life());
    
    patch_starting_flags(world, options);
    patch_items(world, options);
    patch_entity_types(world, options);
    patch_game_strings(world, options);
}